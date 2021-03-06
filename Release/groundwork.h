#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <io.h>
#include "resource.h"
#include "sound.h"
using namespace std;


struct SimpleVertex
	{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
	XMFLOAT3 Norm;
	};


class ConstantBuffer
	{
	public:
		ConstantBuffer()
			{
			info = XMFLOAT4(1, 1, 1, 1);
			}
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX LightView;
	XMFLOAT4 info;
	XMFLOAT4 CameraPos;
	};


//********************************************
//********************************************
class StopWatchMicro_
	{
	private:
		LARGE_INTEGER last, frequency;
	public:
		StopWatchMicro_()
			{
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&last);

			}
		long double elapse_micro()
			{
			LARGE_INTEGER now, dif;
			QueryPerformanceCounter(&now);
			dif.QuadPart = now.QuadPart - last.QuadPart;
			long double fdiff = (long double)dif.QuadPart;
			fdiff /= (long double)frequency.QuadPart;
			return fdiff*1000000.;
			}
		long double elapse_milli()
			{
			return elapse_micro() / 1000.;
			}
		void start()
			{
			QueryPerformanceCounter(&last);
			}
	};
//**********************************
class billboard
	{
	public:
		billboard()
			{
			position = XMFLOAT3(0, 0, 0);
			scale = 1;
			transparency = 1;
			}
		XMFLOAT3 position; //obvious
		float scale;		//in case it can grow
		float transparency; //for later use
		XMMATRIX get_matrix(XMMATRIX &ViewMatrix)
			{

			XMMATRIX view,R, T, S;
			view = ViewMatrix;
			//eliminate camera translation:
			view._41 = view._42 = view._43 = 0.0;
			XMVECTOR det;
			R = XMMatrixInverse(&det, view);//inverse rotation
			T = XMMatrixTranslation(position.x, position.y, position.z);
			S = XMMatrixScaling(scale, scale, scale);
			return S*R*T;
			}

		XMMATRIX get_matrix_y(XMMATRIX &ViewMatrix) //enemy-type
			{

			}
	};

//*****************************************
class bitmap
	{

	public:
		BYTE *image;
		int array_size;
		BITMAPFILEHEADER bmfh;
		BITMAPINFOHEADER bmih;
		bitmap()
			{
			image = NULL;
			}
		~bitmap()
			{
			if(image)
				delete[] image;
			array_size = 0;
			}
		bool read_image(char *filename)
			{
			ifstream bmpfile(filename, ios::in | ios::binary);
			if (!bmpfile.is_open()) return FALSE;	// Error opening file
			bmpfile.read((char*)&bmfh, sizeof(BITMAPFILEHEADER));
			bmpfile.read((char*)&bmih, sizeof(BITMAPINFOHEADER));
			bmpfile.seekg(bmfh.bfOffBits, ios::beg);
			//make the array
			if (image)delete[] image;
			int size = bmih.biWidth*bmih.biHeight * 3;
			image = new BYTE[size];//3 because red, green and blue, each one byte
			bmpfile.read((char*)image,size);
			array_size = size;
			bmpfile.close();
			check_save();
			return TRUE;
			}
		BYTE get_pixel(int x, int y,int color_offset) //color_offset = 0,1 or 2 for red, green and blue
			{
			int array_position = x*3 + y* bmih.biWidth*3+ color_offset;
			if (array_position >= array_size) return 0;
			if (array_position < 0) return 0;
			return image[array_position];
			}
		void check_save()
			{
			ofstream nbmpfile("newpic.bmp", ios::out | ios::binary);
			if (!nbmpfile.is_open()) return;
			nbmpfile.write((char*)&bmfh, sizeof(BITMAPFILEHEADER));
			nbmpfile.write((char*)&bmih, sizeof(BITMAPINFOHEADER));
			//offset:
			int rest = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
			if (rest > 0)
				{
				BYTE *r = new BYTE[rest];
				memset(r, 0, rest);
				nbmpfile.write((char*)&r, rest);
				}
			nbmpfile.write((char*)image, array_size);
			nbmpfile.close();

			}
	};
////////////////////////////////////////////////////////////////////////////////
//lets assume a wall is 10/10 big!
#define FULLWALL 2
#define HALFWALL 1
class wall
	{
	public:
		XMFLOAT3 position;
			int texture_no;
			int rotation; //0,1,2,3,4,5 ... facing to z, x, -z, -x, y, -y
			wall()
				{
				texture_no = 0;
				rotation = 0;
				position = XMFLOAT3(0,0,0);
				}
			XMMATRIX get_matrix()
				{
				XMMATRIX R, T, T_offset;				
				R = XMMatrixIdentity();
				T_offset = XMMatrixTranslation(0, 0, -HALFWALL);
				T = XMMatrixTranslation(position.x, position.y, position.z);
				switch (rotation)//0,1,2,3,4,5 ... facing to z, x, -z, -x, y, -y
					{
						default:
						case 0:	R = XMMatrixRotationY(XM_PI);		T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
						case 1: R = XMMatrixRotationY(XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
						case 2:										T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
						case 3: R = XMMatrixRotationY(-XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, HALFWALL); break;
						case 4: R = XMMatrixRotationX(XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, -HALFWALL); break;
						case 5: R = XMMatrixRotationX(-XM_PIDIV2);	T_offset = XMMatrixTranslation(0, 0, -HALFWALL); break;
					}
				return T_offset * R * T;
				}
	};
//********************************************************************************************
class level
	{
	private:
		bitmap leveldata;
		vector<wall*> walls;						//all wall positions
		vector<ID3D11ShaderResourceView*> textures;	//all wall textures
		void process_level()
			{
			//we have to get the level to the middle:
			int x_offset = (leveldata.bmih.biWidth/2)*FULLWALL;

			//lets go over each pixel without the borders!, only the inner ones
			for (int yy = 1; yy < (leveldata.bmih.biHeight - 1);yy++)
				for (int xx = 1; xx < (leveldata.bmih.biWidth - 1); xx++)
					{
					//wall information is the interface between pixels:
					//blue to something not blue: wall. texture number = 255 - blue
					//green only: floor. texture number = 255 - green
					//red only: ceiling. texture number = 255 - red
					//green and red: floor and ceiling ............
					BYTE red, green, blue;

					blue = leveldata.get_pixel(xx, yy, 0);
					green = leveldata.get_pixel(xx, yy, 1);
					red = leveldata.get_pixel(xx, yy, 2);
					
					if (blue > 0)//wall possible
						{
						int texno = 255 - blue;
						BYTE left_red = leveldata.get_pixel(xx - 1, yy, 2);
						BYTE left_green = leveldata.get_pixel(xx - 1, yy, 1);
						BYTE right_red = leveldata.get_pixel(xx + 1, yy, 2);
						BYTE right_green = leveldata.get_pixel(xx + 1, yy, 1);
						BYTE top_red = leveldata.get_pixel(xx, yy+1, 2);
						BYTE top_green = leveldata.get_pixel(xx, yy+1, 1);
						BYTE bottom_red = leveldata.get_pixel(xx, yy-1, 2);
						BYTE bottom_green = leveldata.get_pixel(xx, yy-1, 1);

						if (left_red>0 || left_green > 0)//to the left
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 3, texno);
						if (right_red>0 || right_green > 0)//to the right
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 1, texno);
						if (top_red>0 || top_green > 0)//to the top
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 2, texno);
						if (bottom_red>0 || bottom_green > 0)//to the bottom
							init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0, yy*FULLWALL), 0, texno);
						}
					if (red > 0)//ceiling
						{
						int texno = 255 - red;
						init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0,yy*FULLWALL), 5, texno);
						}
					if (green > 0)//floor
						{
						int texno = 255 - green;
						init_wall(XMFLOAT3(xx*FULLWALL - x_offset, 0,yy*FULLWALL), 4, texno);
						}
					}
			}
		void init_wall(XMFLOAT3 pos, int rotation, int texture_no)
			{
			wall *w = new wall;
			walls.push_back(w);
			w->position = pos;
			w->rotation = rotation;
			w->texture_no = texture_no;
			}
	public:
		level()
			{
			}
		void init(char *level_bitmap)
			{
			if(!leveldata.read_image(level_bitmap))return;
			process_level();
			}
		bool init_texture(ID3D11Device* pd3dDevice,LPCWSTR filename)
			{
			// Load the Texture
			ID3D11ShaderResourceView *texture;
			HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, filename, NULL, NULL, &texture, NULL);
			if (FAILED(hr))
				return FALSE;
			textures.push_back(texture);
			return TRUE;
			}
		ID3D11ShaderResourceView *get_texture(int no)
			{
			if (no < 0 || no >= textures.size()) return NULL;
			return textures[no];
			}
		XMMATRIX get_wall_matrix(int no)
			{
			if (no < 0 || no >= walls.size()) return XMMatrixIdentity();
			return walls[no]->get_matrix();
			}
		int get_wall_count()
			{
			return walls.size();
			}
		void render_level(ID3D11DeviceContext* ImmediateContext,ID3D11Buffer *vertexbuffer_wall,XMMATRIX *view, XMMATRIX *projection, ID3D11Buffer* dx_cbuffer)
			{
			//set up everything for the waqlls/floors/ceilings:
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;			
			ImmediateContext->IASetVertexBuffers(0, 1, &vertexbuffer_wall, &stride, &offset);
			ConstantBuffer constantbuffer;			
			constantbuffer.View = XMMatrixTranspose(*view);
			constantbuffer.Projection = XMMatrixTranspose(*projection);			
			XMMATRIX wall_matrix,S;
			ID3D11ShaderResourceView* tex;
			//S = XMMatrixScaling(FULLWALL, FULLWALL, FULLWALL);
			S = XMMatrixScaling(1, 1, 1);
			for (int ii = 0; ii < walls.size(); ii++)
				{
				wall_matrix = walls[ii]->get_matrix();
				int texno = walls[ii]->texture_no;
				if (texno >= textures.size())
					texno = 0;
				tex = textures[texno];
				wall_matrix = wall_matrix;// *S;

				constantbuffer.World = XMMatrixTranspose(wall_matrix);
				
				ImmediateContext->UpdateSubresource(dx_cbuffer, 0, NULL, &constantbuffer, 0, 0);
				ImmediateContext->VSSetConstantBuffers(0, 1, &dx_cbuffer);
				ImmediateContext->PSSetConstantBuffers(0, 1, &dx_cbuffer);
				ImmediateContext->PSSetShaderResources(0, 1, &tex);
				ImmediateContext->Draw(6, 0);
				}
			}
	};



	class camera
		{
		private:

		public:
			int w, s, a, d,q,e;
			XMFLOAT3 position;
			XMFLOAT3 rotation;
			XMFLOAT3 impulse;
			XMFLOAT3 impulseActual;
			XMMATRIX Ry, Rx, T;

			float decayRate; // how fast impulse decays to 0.0
			float decayDiff; // used to tell how close to 0.0

			bool fireFoward; //impulse directions
			camera()
				{
				w = s = a = d = 0;
				position = position = XMFLOAT3(0, 0, 0);
				impulse = impulse = XMFLOAT3(0, 0, 0);
				impulseActual = impulseActual = XMFLOAT3(0, 0, 0);

				decayRate = .001;
				decayDiff = 0.00001;

				fireFoward = true;
				}
			void animation(float elapsed_microseconds)
				{
				XMMATRIX Ry,Rx, T;
				Ry = XMMatrixRotationY(-rotation.y);
				Rx = XMMatrixRotationX(-rotation.x);

				XMFLOAT3 forward = XMFLOAT3(0, 0, 1);
				XMVECTOR f = XMLoadFloat3(&forward);
				f = XMVector3TransformCoord(f, Rx*Ry);
				XMStoreFloat3(&forward, f);
				XMFLOAT3 side = XMFLOAT3(1, 0, 0);
				XMVECTOR si = XMLoadFloat3(&side);
				si = XMVector3TransformCoord(si, Rx*Ry);
				XMStoreFloat3(&side, si);

				float speed = elapsed_microseconds / 100000.0;
				//wasd method for debug
				/*if (w)
					{
					position.x -= forward.x * speed;
					position.y -= forward.y * speed;
					position.z -= forward.z * speed;
					}
				if (s)
					{
					position.x += forward.x * speed;
					position.y += forward.y * speed;
					position.z += forward.z * speed;
					}
				if (d)
					{
					position.x -= side.x * 0.01;
					position.y -= side.y * 0.01;
					position.z -= side.z * 0.01;
					}
				if (a)
					{
					position.x += side.x * 0.01;
					position.y += side.y * 0.01;
					position.z += side.z * 0.01;
					}
					}*/
				//mouse click for game
				if (w) {
					impulse.x = forward.x * speed;
					impulse.y = forward.y * speed;
					impulse.z = forward.z * speed;
					if (fireFoward) {
						impulseActual.x -= impulse.x / 4;
						impulseActual.y -= impulse.y / 4;
						impulseActual.z -= impulse.z / 4;
					}
					else {
						impulseActual.x += impulse.x / 4;
						impulseActual.y += impulse.y / 4;
						impulseActual.z += impulse.z / 4;
					}
					//applying to current impulse
				}
					position.x -= impulseActual.x;
					position.y -= impulseActual.y;
					position.z -= impulseActual.z;
					

					if (impulseActual.x > decayDiff) {
						impulseActual.x -= decayRate;
					}

					if (impulseActual.y >decayDiff) {
						impulseActual.y -= decayRate;
					}
					if (impulseActual.z > decayDiff) {
						impulseActual.z -= decayRate;
					}

					if (impulseActual.x <decayDiff) {
						impulseActual.x += decayRate;
					}

					if (impulseActual.y <decayDiff) {
						impulseActual.y += decayRate;
					}
					if (impulseActual.z < decayDiff) {
						impulseActual.z += decayRate;
					}



					//TODO make impulse always decay to 0.0f (if below or below or above.
				
				}
			XMMATRIX get_matrix(XMMATRIX *view)
				{
				XMMATRIX Rx,Ry, T;
				Rx = XMMatrixRotationX(rotation.x);
				Ry = XMMatrixRotationY(rotation.y);
				T = XMMatrixTranslation(position.x, position.y, position.z);
				return T*(*view)*Ry*Rx;
				}
			void fireFoward_flip() {
				fireFoward = !fireFoward; //reserve directions
			}
			//used to UI impulse
			XMFLOAT3 getImpulse() {
				return impulseActual;

			}
		};




	class bullet
		{
		public:
			XMFLOAT3 pos, imp;
			bullet()
				{
				pos = imp = XMFLOAT3(0, 0, 0);
				}
			XMMATRIX getmatrix(float elapsed, XMMATRIX &view)
				{

				pos.x = pos.x + imp.x *(elapsed / 100000.0);
				pos.y = pos.y + imp.y *(elapsed / 100000.0);
				pos.z = pos.z + imp.z *(elapsed / 100000.0);

				XMMATRIX R, T;
				R = view;
				R._41 = R._42 = R._43 = 0.0;
				XMVECTOR det;
				R = XMMatrixInverse(&det, R);
				T = XMMatrixTranslation(pos.x, pos.y, pos.z);

				return R * T;
				}
		};


	class Mine
	{
	public:
		XMFLOAT3 pos, imp;
		bool activated; //flip to change textures
		float explodedTime;//used to determined with to explod.
		Mine()
		{
			pos = imp = XMFLOAT3(0, 0, 0);
			activated = false;
			explodedTime = .0f;
		}
		Mine(XMFLOAT3 apos) {
			pos = apos;
			activated = false;
			explodedTime = .0f;

		}

		XMMATRIX getmatrix(float elapsed, XMMATRIX &view)
		{

			pos.x = pos.x + imp.x *(elapsed / 100000.0);
			pos.y = pos.y + imp.y *(elapsed / 100000.0);
			pos.z = pos.z + imp.z *(elapsed / 100000.0);

			XMMATRIX R, T;
			R = view;
			R._41 = R._42 = R._43 = 0.0;
			XMVECTOR det;
			R = XMMatrixInverse(&det, R);
			T = XMMatrixTranslation(pos.x, pos.y, pos.z);

			return R * T;
		}
		void activate(float elapsed) {
			activated = true;
			explodedTime = elapsed;
		}
		bool explode(float elapsed) {
		//takes differences between activatioTime if more then 2 seconds, explods.
			if (elapsed - explodedTime > 10000 && activated)
				return true;
			else
				return false;
		
		}


	};


	class TrackerMine
	{
	public:
		XMFLOAT3 pos, imp, rot;
		bool activated;
		TrackerMine()
		{
			pos = imp = XMFLOAT3(0, 0, 0);
			activated = false;
		}
		TrackerMine(XMFLOAT3 apos) {
			pos = apos;
			activated = false;
			

		}
		XMMATRIX getmatrix(float elapsed, XMMATRIX &view)
		{
		
			pos.x = pos.x + imp.x *(elapsed / 100000.0);
			pos.y = pos.y + imp.y *(elapsed / 100000.0);
			pos.z = pos.z + imp.z *(elapsed / 100000.0);

			XMMATRIX R, T;
			R = view;
			R._41 = R._42 = R._43 = 0.0;
			XMVECTOR det;
			R = XMMatrixInverse(&det, R);
			T = XMMatrixTranslation(pos.x, pos.y, pos.z);

			return R * T;
		}
		void animate(XMFLOAT3 g, float elapsed_microseconds) {
			XMMATRIX Ry, Rx, T;
			
			pos.x -= imp.x / 1000;
			pos.y -= imp.y / 1000;
			pos.z -= imp.z / 1000;

		}


	};

	float Vec3Length(const XMFLOAT3 &v);
	float Vec3Dot(XMFLOAT3 a, XMFLOAT3 b);
	XMFLOAT3 Vec3Cross(XMFLOAT3 a, XMFLOAT3 b);
	XMFLOAT3 Vec3Normalize(const  XMFLOAT3 &a);
	XMFLOAT3 operator+(const XMFLOAT3 lhs, const XMFLOAT3 rhs);
	XMFLOAT3 operator-(const XMFLOAT3 lhs, const XMFLOAT3 rhs);
	bool Load3DS(char *filename, ID3D11Device* g_pd3dDevice, ID3D11Buffer **ppVertexBuffer, int *vertex_count);
	bool LoadCMP(LPCTSTR filename, ID3D11Device* g_pd3dDevice, ID3D11Buffer **ppVertexBuffer, int *vertex_count);