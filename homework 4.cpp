//--------------------------------------------------------------------------------------
// File: lecture 8.cpp
//
//File: CST320 Final Project: Celesterial Drift
//Authors: Michael, Miguel, Peter
//File: lecture 8.cpp
// This application demonstrates texturing
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "render_to_texture.h"
#include "Font.h"


CXBOXController *gamepad = NULL;






//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
ID3D11Texture2D*                    g_pDepthStencil2 = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView2 = NULL;
int fact = 2;
ID3D11VertexShader*                 g_pVertexShader = NULL;
ID3D11PixelShader*                  g_pPixelShader = NULL;
RenderTextureClass					RenderToTexture;
RenderTextureClass					DepthLight;
ID3D11VertexShader*                 g_pVertexShader_screen = NULL;
ID3D11PixelShader*                  g_pPixelShader_screen = NULL;
ID3D11PixelShader*                  PSdepth = NULL;

ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer = NULL;
ID3D11Buffer*                       g_pVertexBuffer_screen = NULL;
ID3D11Buffer*                       g_pVertexBuffer_sky = NULL;
ID3D11Buffer*                       g_pVertexBuffer_3ds = NULL;
int									model_vertex_anz = 0;

//MODELS

//astroid
ID3D11Buffer*                       g_pVertexBuffer_3ds_asteroids = NULL;
int									model_vertex_anz_asteroids = 0;
ID3D11ShaderResourceView*           g_pTexture_asteroid = NULL;
#define ASTEROIDCOUNT				100
XMFLOAT4 asteroid_pos[2000];




//instance Rendering
ID3D11VertexShader*                 g_pInstanceShader = NULL;
ID3D11InputLayout*                  g_pInstanceLayout = NULL;
ID3D11Buffer*                       g_pInstancebuffer = NULL;


//mines
#define MINECOUNT					50
vector<Mine*>						StationaryMines;



//navigation arrow
ID3D11Buffer*                       g_pVertexBuffer_3ds_nav = NULL;
int									model_vertex_anz_nav = 0;


// Sky Sphere
ID3D11Buffer*						g_pVertexBuffer_cmp;
int									model_vertex_anz_sky;
ID3D11ShaderResourceView*           g_pTexture_sky = NULL;

//Space Station
ID3D11Buffer*						g_pVertexBuffer_ss;
int									model_vertex_anz_ss;
ID3D11ShaderResourceView*           g_pTexture_ss = NULL;


//states for turning off and on the depth buffer
ID3D11DepthStencilState				*ds_on, *ds_off;
ID3D11BlendState*					g_BlendState;

ID3D11Buffer*                       g_pCBuffer = NULL;


//TEXTURES
ID3D11ShaderResourceView*           g_pTextureRV = NULL;
ID3D11ShaderResourceView*           g_pTextureNav = NULL; //nav arrow

ID3D11RasterizerState				*rs_CW, *rs_CCW, *rs_NO, *rs_Wire;

ID3D11SamplerState*                 g_pSamplerLinear = NULL;
ID3D11SamplerState*                 SamplerScreen = NULL;

XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;
XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );

camera								cam;
level								level1;
vector<billboard*>					smokeray;
XMFLOAT3							rocket_position;

//powerup objects
vector<XMFLOAT3*>					oneUps;



//movment variables

static StopWatchMicro_				fireTimer;
bool								fireFoward = true; //used to switch movment dictions, true = shoot forward, fly backwards, false, = reverse 
bool								canFire = true;

// globals for game balance
XMFLOAT3							objectivePos = XMFLOAT3(1, 1, 1);//used to nav arrow to point to object cords

int									fireDelay = 200; // in milliseconds, delay between fire(.5 seconds = 500).
int									fireReserveDelay = 200; // in milliseconds, delay between switching fire directions(.5 seconds = 500).
int									playerLives;
int									playerField = 1000;//used to determine how far the player can go. 



															//rail gun
vector<bullet*>						bullets;
XMFLOAT3							bullet_position;

//Font
Font								font;
string								reload = "";

//Sound
music_								sound;

explosion_handler  explosionhandler;



#define ROCKETRADIUS				10
//--------------------------------------------------------------------------------------
// Forward declarxzations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();

//--------------------------------------------------------------------------------------
// Extras
//--------------------------------------------------------------------------------------
float frand()
{
	int r = rand();
	float res = (float)r / (float)RAND_MAX;
	return res;
}
void playerDeath() {
	//if player collids with mine, or astroid then -1 life. functions checks if this fall below zero, if so ends game. TODO change to change game state.
	playerLives--;
	if (playerLives < 1) {
		PostQuitMessage(1);
	}
	return;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }
	srand(time(0));
    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1024, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 7", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
		{
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
		}
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
		return hr;

	
    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width*fact;
	descDepth.Height = height*fact;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil2);
	if (FAILED(hr))
		return hr;


	// Create the depth stencil view
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil2, &descDSV, &g_pDepthStencilView2);
	if (FAILED(hr))
		return hr;

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

	

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile( L"shader.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
        return hr;
    }

	// Compile the vertex shader
	pVSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "VS_screen", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
		{
		MessageBox(NULL,
				   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
		}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader_screen);
	if (FAILED(hr))
		{
		pVSBlob->Release();
		return hr;
		}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	pVSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "VS_instance", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pInstanceShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutInstance[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEVEC", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "ROTATEINST", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

	};
	numElements = ARRAYSIZE(layoutInstance);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layoutInstance, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pInstanceLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pInstanceLayout);

	
	//randomizing astroids
	for (int ii = 0; ii < ASTEROIDCOUNT * 2; ii += 2)
	{
		float x, y, z, w;
		w = rand() % 50 - 25;
		z = rand() % 1000 - 500;
		x = rand() % 1000 - 500;
		y = rand() % 1000 - 500;
		while (x*x+y*y+z*z <= 1600)
		{
			z = rand() % 1000 - 500;
			x = rand() % 1000 - 500;
			y = rand() % 1000 - 500;
		}
		asteroid_pos[ii] = XMFLOAT4(x, y, z, w);
	}
	for (int ii = 1; ii < ASTEROIDCOUNT * 2; ii += 2)
	{
		float x, y, z, w;
		w = (frand()*2.0 - 1.0);
		x = (frand()*2.0 - 1.0) * XM_PI*2.0;
		y = (frand()*2.0 - 1.0) * XM_PI*2.0;
		z = (frand()*2.0 - 1.0) * XM_PI*2.0;
		asteroid_pos[ii] = XMFLOAT4(x, y, z, w);
	}


	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(XMFLOAT4)* ASTEROIDCOUNT * 2;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = (BYTE*)asteroid_pos;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pInstancebuffer);
	if (FAILED(hr))
		return hr;

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"shader.fx", "PS", "ps_5_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;


	// Compile the pixel shader
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "PS_screen", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
		{
		MessageBox(NULL,
				   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
		}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader_screen);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;



	// Compile the pixel shader
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "PSdepth", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
		{
		MessageBox(NULL,
				   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
		}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PSdepth);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	
	//create skybox vertex buffer
	
	
	
    // Create vertex buffer
	SimpleVertex vertices[] =
		{
				{ XMFLOAT3(-1,1,0),XMFLOAT2(0,0),XMFLOAT3(0,0,-1) },
				{ XMFLOAT3(1,1,0),XMFLOAT2(1,0),XMFLOAT3(0,0,-1) },
				{ XMFLOAT3(-1,-1,0),XMFLOAT2(0,1),XMFLOAT3(0,0,-1) },
				{ XMFLOAT3(1,1,0),XMFLOAT2(1,0),XMFLOAT3(0,0,-1) },
				{ XMFLOAT3(1,-1,0),XMFLOAT2(1,1),XMFLOAT3(0,0,-1) },
				{ XMFLOAT3(-1,-1,0),XMFLOAT2(0,1),XMFLOAT3(0,0,-1) }
		};
	
	//initialize d3dx verexbuff:
	bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	 InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_screen);
	if (FAILED(hr))
		return FALSE;
	
    ZeroMemory( &bd, sizeof(bd) );
   
	//load model 3ds file

	Load3DS("asteroid.3ds", g_pd3dDevice, &g_pVertexBuffer_3ds_asteroids, &model_vertex_anz_asteroids);
	
	//loading nav arrow
	Load3DS("nav_arrow.3ds", g_pd3dDevice, &g_pVertexBuffer_3ds_nav, &model_vertex_anz_nav);

	//Load Sky Sphere
	LoadCMP(L"ccsphere.cmp", g_pd3dDevice, &g_pVertexBuffer_cmp, &model_vertex_anz_sky);

	//Load Space station
	LoadCMP(L"station.3ds", g_pd3dDevice, &g_pVertexBuffer_ss, &model_vertex_anz_ss);

	//randomizing the mine position
	Mine * tm; 
	for (int ii = 0; ii < MINECOUNT; ii++){
		float x, y, z, w;
		z = rand() % 1000 - 100;
		x = rand() % 1000 - 100;
		y = rand() % 1000 - 100;
		tm = new Mine(XMFLOAT3(x, y, z));

		StationaryMines.push_back(tm);

	}
	
	//randomizing the one ups
	XMFLOAT3* ps;
	for (int i = 0; i < 20; i++) {
		float x, y, z;
		z = rand() % 1000 - 100;
		x = rand() % 1000 - 100;
		y = rand() % 1000 - 100;
		ps = new XMFLOAT3(x, y, z);
		oneUps.push_back(ps);
		
	}
	//randominging the space station
	float px, py, pz;

	pz = rand() % 1000 - 100;
	px = rand() % 1000 - 100;
	py = rand() % 1000 - 100;
	objectivePos.x = px;
	objectivePos.x = pz;
	objectivePos.x = py;

	while (px*px + py*py + pz*pz <= 5000)
		{
		pz = rand() % 1000 - 500;
		px = rand() % 1000 - 500;
		py = rand() % 1000 - 500;
		 }




    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

 
    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBuffer);
    if( FAILED( hr ) )
        return hr;
    

    // Load the Texture
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"asteroid_1.png", NULL, NULL, &g_pTexture_asteroid, NULL );
    if( FAILED( hr ) )
        return hr;

	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"space.png", NULL, NULL, &g_pTexture_sky, NULL);
	if (FAILED(hr))
		return hr;

	// Load the nav arrow
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"nav_arrow_tex.png", NULL, NULL, &g_pTextureNav, NULL);
	if (FAILED(hr))
		return hr;

	//setting player lives
	playerLives = 1;

	//randomzing one ups

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;

	// Create the screen sample state
	
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &SamplerScreen);
	if (FAILED(hr))
		return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );//camera position
    XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );//look at
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );// normal vector on at vector (always up)
    g_View = XMMatrixLookAtLH( Eye, At, Up );

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100000.0f);

	ConstantBuffer constantbuffer;
	constantbuffer.View = XMMatrixTranspose( g_View );
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.World = XMMatrixTranspose(XMMatrixIdentity());
	constantbuffer.info = XMFLOAT4(1, 1, 1, 1);
    g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0 );

	//blendstate:
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	g_pd3dDevice->CreateBlendState(&blendStateDesc, &g_BlendState);


	float blendFactor[] = { 0, 0, 0, 0 };
	UINT sampleMask = 0xffffffff;
	g_pImmediateContext->OMSetBlendState(g_BlendState, blendFactor, sampleMask);
	

	//create the depth stencil states for turning the depth buffer on and of:
	D3D11_DEPTH_STENCIL_DESC		DS_ON, DS_OFF;
	DS_ON.DepthEnable = true;
	DS_ON.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DS_ON.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	DS_ON.StencilEnable = true;
	DS_ON.StencilReadMask = 0xFF;
	DS_ON.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	DS_ON.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DS_ON.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	DS_ON.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DS_ON.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil state
	DS_OFF = DS_ON;
	DS_OFF.DepthEnable = false;
	g_pd3dDevice->CreateDepthStencilState(&DS_ON, &ds_on);
	g_pd3dDevice->CreateDepthStencilState(&DS_OFF, &ds_off);

	level1.init("level.bmp");
	level1.init_texture(g_pd3dDevice, L"wall1.jpg");
	level1.init_texture(g_pd3dDevice, L"wall2.jpg");
	level1.init_texture(g_pd3dDevice, L"floor.jpg");
	level1.init_texture(g_pd3dDevice, L"ceiling.jpg");
	
	rocket_position = XMFLOAT3(0, 0, ROCKETRADIUS);

	// fire limiter
	fireTimer.start();//starting timer
	
	//font stuff
	font.init(g_pd3dDevice, g_pImmediateContext, font.defaultFontMapDesc);

	//setting the rasterizer:
	D3D11_RASTERIZER_DESC			RS_CW, RS_Wire;

	RS_CW.AntialiasedLineEnable = FALSE;
	RS_CW.CullMode = D3D11_CULL_BACK;
	RS_CW.DepthBias = 0;
	RS_CW.DepthBiasClamp = 0.0f;
	RS_CW.DepthClipEnable = true;
	RS_CW.FillMode = D3D11_FILL_SOLID;
	RS_CW.FrontCounterClockwise = false;
	RS_CW.MultisampleEnable = FALSE;
	RS_CW.ScissorEnable = false;
	RS_CW.SlopeScaledDepthBias = 0.0f;

	RS_Wire = RS_CW;
	RS_Wire.CullMode = D3D11_CULL_NONE;
	RS_Wire.FillMode = D3D11_FILL_WIREFRAME;
	g_pd3dDevice->CreateRasterizerState(&RS_Wire, &rs_Wire);
	g_pd3dDevice->CreateRasterizerState(&RS_CW, &rs_CW);

	//rendertarget texture
	RenderToTexture.Initialize(g_pd3dDevice, g_hWnd, -1, -1, FALSE, DXGI_FORMAT_R8G8B8A8_UNORM, TRUE);

	DepthLight.Initialize(g_pd3dDevice, g_hWnd, -2, -1, FALSE, DXGI_FORMAT_R32G32B32A32_FLOAT, TRUE);


	hr=explosionhandler.init(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
		return hr;
	hr = explosionhandler.init_types(L"exp1.dds", 8, 8,1000000);
	if (FAILED(hr))
		return hr;
	hr = explosionhandler.init_types(L"exp2.dds", 9, 9,1500000);
	if (FAILED(hr))
		return hr;
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pSamplerLinear ) g_pSamplerLinear->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if(g_pCBuffer) g_pCBuffer->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is down
///////////////////////////////////
bullet *bull = NULL;
void OnLBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
	/*bull = new bullet;
	bull->pos.x = -cam.position.x;
	bull->pos.y = -cam.position.y-1.2;
	bull->pos.z = -cam.position.z;
	XMMATRIX CR = XMMatrixRotationY(-cam.rotation.y);

	XMFLOAT3 forward = XMFLOAT3(0, 0, 3);
	XMVECTOR f = XMLoadFloat3(&forward);
	f = XMVector3TransformCoord(f, CR);
	XMStoreFloat3(&forward, f);

	bull->imp = forward;

	explosionhandler.new_explosion(XMFLOAT3(0, 0, 10), XMFLOAT3(0, 0, 0), 0, 8.0);*/

	}

///////////////////////////////////
//		This Function is called every time the Right Mouse Button is down
///////////////////////////////////
void OnRBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
	//explosionhandler.new_explosion(XMFLOAT3(0, 0, 11), XMFLOAT3(0, 0, 5), 1, 4.0);
	}
///////////////////////////////////
//		This Function is called every time a character key is pressed
///////////////////////////////////
void OnChar(HWND hwnd, UINT ch, int cRepeat)
	{

	}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is up
///////////////////////////////////
void OnLBU(HWND hwnd, int x, int y, UINT keyFlags)
	{
	
	

	if (canFire) {
		cam.w = 1;
		canFire = false;
		fireTimer.start();//wating .5 secs before you cna fire again
		reload = " ";//resetting fire UI
	
		bull = new bullet;
		bull->pos.x = -cam.position.x;
		bull->pos.y = -cam.position.y - 1.2;
		bull->pos.z = -cam.position.z;
		XMMATRIX CR = cam.get_matrix(&g_View);
		CR._41 = 0;
		CR._42 = 0;
		CR._43 = 0;
		XMVECTOR det;
		XMMATRIX ICR = XMMatrixInverse(&det, CR);
		XMFLOAT3 forward = XMFLOAT3(0, 0, 3);
		XMVECTOR f = XMLoadFloat3(&forward);

		if (fireFoward) {
			f = XMVector3TransformCoord(f, ICR);
		}
		else {
			f = XMVector3TransformCoord(f, CR);
		}

		XMStoreFloat3(&forward, f);
		bull->imp = forward;
		bullets.push_back(bull);
		sound.play_fx("boost.mp3");
		}

	}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is up
///////////////////////////////////
void OnRBU(HWND hwnd, int x, int y, UINT keyFlags)
	{


	}
///////////////////////////////////
//		This Function is called every time the Mouse Moves
///////////////////////////////////
void OnMM(HWND hwnd, int x, int y, UINT keyFlags)
	{
	static int holdx = x, holdy = y;
	static int reset_cursor = 0;



	RECT rc; 			//rectange structure
	GetWindowRect(hwnd, &rc); 	//retrieves the window size
	int border = 20;
	rc.bottom -= border;
	rc.right -= border;
	rc.left += border;
	rc.top += border;
	ClipCursor(&rc);

	if ((keyFlags & MK_LBUTTON) == MK_LBUTTON)
		{
		}

	if ((keyFlags & MK_RBUTTON) == MK_RBUTTON)
		{
		}
	if (reset_cursor == 1)
		{		
		reset_cursor = 0;
		holdx = x;
		holdy = y;
		return;
		}
	int diffx = holdx - x;
	int diffy = holdy - y;
	float angle_y = (float)diffx / 300.0;
	float angle_x = (float)diffy / 300.0;
	cam.rotation.y += angle_y;
	cam.rotation.x += angle_x;

	int midx = (rc.left + rc.right) / 2;
	int midy = (rc.top + rc.bottom) / 2;
	SetCursorPos(midx, midy);
	reset_cursor = 1;
	}

unsigned int Timer = -1;
BOOL OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
	{
	RECT rc; 			//rectange structure
	GetWindowRect(hwnd, &rc); 	//retrieves the window size
	int border = 5;
	rc.bottom -= border;
	rc.right -= border;
	rc.left += border;
	rc.top += border;
	ClipCursor(&rc);
	int midx = (rc.left + rc.right) / 2;
	int midy = (rc.top + rc.bottom) / 2;
	SetCursorPos(midx,midy);

	if (!SetTimer(hwnd, Timer, 5, NULL))
		{
		return FALSE;
		}
	gamepad = new CXBOXController(1);

	return TRUE;
	}
//---------------------------------------
void OnTimer(HWND hwnd, UINT id)
	{
	if (gamepad->IsConnected())
		{
		if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y)
			cam.w = 1;
		else
			cam.w = 0;
		if (gamepad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A)
			{
			
			cam.s = 1;
			}
		else
			cam.s = 0;

		}
	SHORT lx = gamepad->GetState().Gamepad.sThumbLX;
	SHORT ly = gamepad->GetState().Gamepad.sThumbLY;

	if (abs(ly) > 3000)
		{
		float angle_x = (float)ly / 32000.0;
		angle_x *= 0.05;
		cam.rotation.x -= angle_x;
		}
	if (abs(lx) > 3000)
		{
		float angle_y = (float)lx / 32000.0;
		angle_y *= 0.05;
		cam.rotation.y -= angle_y;
		}
	
	}
//*************************************************************************
void OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
	switch (vk)
		{
			case 81://q
			cam.q = 0; break;
			case 69://e
			cam.e = 0; break;
			case 65:cam.a = 0;//a
				break;
			case 68: cam.d = 0;//d
				break;
			case 32: //space
				cam.fireFoward_flip();
				fireFoward = !fireFoward;
				break;
			case 87: cam.w = 0; //w
				break;
			case 83:cam.s = 0; //s
			default:break;

		}

	}

void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{

	switch (vk)
		{
			default:break;
			case 81://q
			cam.q = 1; break;
			case 69://e
			cam.e = 1; break;
			case 65:cam.a = 1;//a
				break;
			case 68: cam.d = 1;//d
				break;
			case 32: //space
			break;
			case 87: cam.w = 1; //w
				break;
			case 83:cam.s = 1; //s
				break;
			case 27: PostQuitMessage(0);//escape
				break;

			case 84://t
			{
			static int laststate = 0;
			if (laststate == 0)
				{
				g_pImmediateContext->RSSetState(rs_Wire);
				laststate = 1;
				}
			else
				{
				g_pImmediateContext->RSSetState(rs_CW);
				laststate = 0;
				}

			}
			break;

		}
	}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
#include <windowsx.h>
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
	HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLBD);
	HANDLE_MSG(hWnd, WM_RBUTTONDOWN, OnRBD);
	HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLBU);
	HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMM);
	HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
	HANDLE_MSG(hWnd, WM_TIMER, OnTimer);
	HANDLE_MSG(hWnd, WM_KEYDOWN, OnKeyDown);
	HANDLE_MSG(hWnd, WM_KEYUP, OnKeyUp);
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}



//--------------------------------------------------------------------------------------
// sprites
//--------------------------------------------------------------------------------------
class sprites
	{
	public:
		XMFLOAT3 position;
		XMFLOAT3 impulse;
		float rotation_x;
		float rotation_y;
		float rotation_z;
		sprites()
			{
			impulse = position = XMFLOAT3(0, 0, 0);
			rotation_x = rotation_y = rotation_z;
			}
		XMMATRIX animation() 
			{
			//update position:
			position.x = position.x + impulse.x; //newtons law
			position.y = position.y + impulse.y; //newtons law
			position.z = position.z + impulse.z; //newtons law

			XMMATRIX M;
			//make matrix M:
			XMMATRIX R,Rx,Ry,Rz,T;
			T = XMMatrixTranslation(position.x, position.y, position.z);
			Rx = XMMatrixRotationX(rotation_x);
			Ry = XMMatrixRotationX(rotation_y);
			Rz = XMMatrixRotationX(rotation_z);
			R = Rx*Ry*Rz;
			M = R*T;
			return M;
			}
	};
sprites mario;

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
TrackerMine a;

//############################################################################################################
void Render_from_light_source(long elapsed)
	{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	ID3D11RenderTargetView*			RenderTarget;
	
	RenderTarget = DepthLight.GetRenderTarget();

	g_pImmediateContext->ClearRenderTargetView(RenderTarget, ClearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView2, D3D11_CLEAR_DEPTH, 1.0, 0);
	g_pImmediateContext->OMSetRenderTargets(1, &RenderTarget, g_pDepthStencilView2);
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width*fact;
	vp.Height = (FLOAT)height*fact;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);


	XMMATRIX view = cam.get_matrix(&g_View);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	// Update constant buffer
	ConstantBuffer constantbuffer;
	XMVECTOR Eye = XMVectorSet(20.0f, 90.0f, 0.0f, 0.0f);//camera position
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);//look at
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);// normal vector on at vector (always up)
	XMMATRIX LightView = XMMatrixLookAtLH(Eye, At, Up);

	constantbuffer.LightView = XMMatrixIdentity();
	constantbuffer.View = XMMatrixTranspose(LightView);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);

	//render model:
	XMMATRIX S = XMMatrixScaling(1, 1, 1);

	S = XMMatrixScaling(1, 1, 1);
	//S = XMMatrixScaling(10, 10, 10);
	XMMATRIX T, R, M;
	T = XMMatrixTranslation(0.1, 0.1, 0.1);
	R = XMMatrixRotationX(-XM_PIDIV2);
	static float angle = 0;
	angle += elapsed / 7000000.0;
	XMMATRIX Ry = XMMatrixRotationY(angle);
	M = S*R*Ry*T;
	constantbuffer.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	// Render terrain
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(PSdepth, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(model_vertex_anz, 0);

	S = XMMatrixScaling(50, 50, 50);
	R = XMMatrixRotationX(XM_PIDIV2);
	T = XMMatrixTranslation(0, -5, 0);
	M = S*R*T;
	constantbuffer.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_screen, &stride, &offset);
	g_pImmediateContext->Draw(6, 0);

	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);
	//
	// Present our back buffer to our front buffer
	//
	g_pSwapChain->Present(0, 0);

	}
//############################################################################################################

//############################################################################################################
void Render_to_texture(long elapsed)
	{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	ID3D11RenderTargetView*			RenderTarget;

	//for delaying fire
	long double curtime = fireTimer.elapse_milli();//if code isn't working, check StopWatchMicro .elapse_milli...
	if (elapsed % 10 == 0 && reload.size() < 10) {
		reload += "|";
	}
	if (curtime > fireDelay) {
		cam.w = 0;
		canFire = true;
	}
	//wating fireDalay secs before you cna fire again


	RenderTarget = RenderToTexture.GetRenderTarget();
	g_pImmediateContext->ClearRenderTargetView(RenderTarget, ClearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);
	g_pImmediateContext->OMSetRenderTargets(1, &RenderTarget, g_pDepthStencilView);

	XMMATRIX view = cam.get_matrix(&g_View);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	// Update constant buffer
	ConstantBuffer constantbuffer;

	XMVECTOR Eye = XMVectorSet(20.0f, 90.0f, 0.0f, 0.0f);//camera position
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);//look at
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);// normal vector on at vector (always up)
	XMMATRIX LightView = XMMatrixLookAtLH(Eye, At, Up);

	constantbuffer.LightView = XMMatrixTranspose(LightView);
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);

	//render model:
	XMMATRIX S = XMMatrixScaling(1, 1, 1);

	S = XMMatrixScaling(1, 1, 1);
	//S = XMMatrixScaling(10, 10, 10);
	XMMATRIX T, R, M;
	T = XMMatrixTranslation(0.1, 0.1, 0.1);
	R = XMMatrixRotationX(-XM_PIDIV2);
	static float angle = 0;
	angle += elapsed / 7000000.0;
	XMMATRIX Ry = XMMatrixRotationY(angle);
	M = S*R*Ry*T;
	constantbuffer.World = XMMatrixTranspose(M);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	// Render terrain
	ID3D11ShaderResourceView*          DepthTexture = DepthLight.GetShaderResourceView();
	g_pImmediateContext->GenerateMips(DepthTexture);
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	g_pImmediateContext->PSSetShaderResources(1, 1, &DepthTexture);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_cmp, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	//g_pImmediateContext->Draw(model_vertex_anz, 0);

	//-----------------------------------------------------------------------------------
	//Sky Sphere
	//-----------------------------------------------------------------------------------
	constantbuffer.World = XMMatrixTranspose(XMMatrixTranslation(-cam.position.x, -cam.position.y, -cam.position.z));
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader_screen, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture_sky);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTexture_sky);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_cmp, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->VSSetSamplers(0, 1, &g_pSamplerLinear);

	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);
	g_pImmediateContext->Draw(model_vertex_anz_sky, 0);
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);

	//-----------------------------------------------------------------------------------
	//NAV ARROW
	//-----------------------------------------------------------------------------------
	XMMATRIX R0, R1, M1, M2,T1, T2, Rx1, Ry1, T3, Rx3, Ry3;
	XMVECTOR cur = XMVector4Normalize(XMVectorSet(-cam.position.x, -cam.position.y + 2.0f, -cam.position.z - 10.0f, 0.0f));//current position
	XMVECTOR goal = XMVector4Normalize(XMVectorSet(1.0,1.0,1.0, 1.0f)); //look at i.e. objective location

	//XMVECTOR cur = XMVectorSet(cam.position.x, cam.position.y, cam.position.z, 0.0f);//current position
	//XMVECTOR goal = XMVectorSet(1.0f,1.0f,1.0f, 1.0f); //look at i.e. objective location
	T = XMMatrixLookAtLH(cur, goal, Up);//used to set where nav arrow points
	R0 = XMMatrixRotationX(-XM_PI);
	T2 = XMMatrixTranslation(0.0f, -2, 10);
	Rx1 = XMMatrixRotationX(-cam.rotation.y);
	Ry1 = XMMatrixRotationY(-cam.rotation.x);
	Rx3 = XMMatrixRotationX(cam.rotation.x);
	Ry3 = XMMatrixRotationY(cam.rotation.y);

	XMMATRIX CR = cam.get_matrix(&g_View);
	CR._41 = 0;
	CR._42 = 0;
	CR._43 = 0;
	XMVECTOR det;
	XMMATRIX ICR = XMMatrixInverse(&det, CR);
	T1 = XMMatrixTranslation(0.0f, -1.0f, 5.0f);
	T3 = XMMatrixTranslation(-cam.position.x, -cam.position.y , -cam.position.z );
	
	R1 = Rx1 * Ry1;
	
	M2 = T * R0 * T2 *ICR * T3;
	constantbuffer.World = XMMatrixTranspose(M2);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureNav);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds_nav, &stride, &offset);
	
	g_pImmediateContext->Draw(model_vertex_anz_nav, 0);
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);



	//end nav arrow


	//-----------------------------------------------------------------------------------
	//BULLETS
	//-----------------------------------------------------------------------------------
	for (int ii = 0; ii < bullets.size(); ii++)
	{
		if (bull != NULL)
		{
			ConstantBuffer constantbuffer;
			XMMATRIX worldmatrix = bull->getmatrix(elapsed, view);
			
			g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureNav);
			constantbuffer.World = XMMatrixTranspose(worldmatrix);
			constantbuffer.View = XMMatrixTranspose(view);
			constantbuffer.Projection = XMMatrixTranspose(g_Projection);
			constantbuffer.Projection = XMMatrixTranspose(g_Projection);
			g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds_nav, &stride, &offset);
			g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
			g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
			g_pImmediateContext->Draw(model_vertex_anz_nav, 0);

		}
	}
	//bullet end




	//-----------------------------------------------------------------------------------
	//Mine rendering
	//-----------------------------------------------------------------------------------
	static float ms = 1.0f;
	ms += .01;
	for (int ii = 0; ii < StationaryMines.size(); ii++)
	{
		//display 
			ConstantBuffer constantbuffer;
		XMMATRIX T = XMMatrixTranslation(StationaryMines[ii]->pos.x, StationaryMines[ii]->pos.y, StationaryMines[ii]->pos.z);
		XMMATRIX S = XMMatrixScaling(20, 20, 20);
		constantbuffer.World = XMMatrixTranspose(S*T);
		constantbuffer.View = XMMatrixTranspose(view);
		constantbuffer.Projection = XMMatrixTranspose(g_Projection);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds_nav, &stride, &offset);
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureNav);
		g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
		g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
		g_pImmediateContext->Draw(model_vertex_anz_nav, 0);

	}

	//-----------------------------------------------------------------------------------
	//Space Station
	//-----------------------------------------------------------------------------------
	S = XMMatrixScaling(1, 1, 1);
	R = XMMatrixRotationX(XM_PIDIV2);
	T = XMMatrixTranslation(0, 0, 100);
	M = S*R*T;
	constantbuffer.World = XMMatrixTranspose(M);
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_ss, &stride, &offset);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture_asteroid);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(model_vertex_anz_ss, 0);


	//-----------------------------------------------------------------------------------
	//One up render
	//-----------------------------------------------------------------------------------
	for (int ii = 0; ii < oneUps.size(); ii++)
	{
		//display
		ConstantBuffer constantbuffer;
		XMMATRIX S = XMMatrixScaling(50, 50, 50);
		XMMATRIX T = XMMatrixTranslation(oneUps[ii]->x, oneUps[ii]->y, oneUps[ii]->z);
		constantbuffer.World = XMMatrixTranspose(S* T);
		constantbuffer.View = XMMatrixTranspose(view);
		constantbuffer.Projection = XMMatrixTranspose(g_Projection);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds_nav, &stride, &offset);
		g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture_asteroid);
		g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
		g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
		g_pImmediateContext->Draw(model_vertex_anz_nav, 0);

	}



	//-----------------------------------------------------------------------------------
	//tracker Mine rendering
	//-----------------------------------------------------------------------------------
	
	
	XMMATRIX TMT, TMR, TMM;
	a.pos = XMFLOAT3(0, 0, 100);
	T = XMMatrixTranslation(a.pos.x, a.pos.y, a.pos.z);
	S = XMMatrixScaling(10, 10, 10);

	XMFLOAT3  v = cam.position - a.pos;
	XMVECTOR V = XMVectorSet(v.x, v.y, v.z, 0.0f);

	V = XMVector4Normalize(V);
	a.animate(v, elapsed);
	//TT = a.getmatrix(elapsed, 
	TMR = a.getmatrix(elapsed, view);
	
	
	
	constantbuffer.World = XMMatrixTranspose(TMR);
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_3ds_nav, &stride, &offset);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture_asteroid);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(model_vertex_anz_nav, 0);


		

	//-----------------------------------------------------------------------------------
	//HEADS UP DISPLAy
	//-----------------------------------------------------------------------------------
	font.setScaling(XMFLOAT3(1.5, 1.5, 1.5));
	font.setColor(XMFLOAT3(21.0, 106.0, 242.0));
	font.setPosition(XMFLOAT3(-.95f, -0.8f, 0.0f));
	font << "FIRE STATUS: ";

	font.setColor(XMFLOAT3(1, .61, 1.58));

	font.setPosition(XMFLOAT3(-.6f, -0.8f, 0.0f));

	if (canFire) { font.setColor(XMFLOAT3(0, 1, .6)); }
	else { font.setColor(XMFLOAT3(1, 0, 0)); }
	font << reload;

	font.setScaling(XMFLOAT3(1.5, 1.5, 1.5));
	font.setColor(XMFLOAT3(21.0, 106.0, 242.0));
	font.setPosition(XMFLOAT3(-0.5, .9, 0));
	font << "Rail Gun Direction: ";

	//font
	font.setScaling(XMFLOAT3(1.5, 1.5, 1.5));

	font.setPosition(XMFLOAT3(-.05, .9, 0));
	if (fireFoward) {
		font.setColor(XMFLOAT3(0, 1, .6));
		font << "FORWARD";
	}
	else {
		font.setColor(XMFLOAT3(1, 0, 0));
		font << "BACKWARD";
		}

	XMFLOAT3 impulseUI = cam.getImpulse();

	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(21.0, 106.0, 242.0));
	font.setPosition(XMFLOAT3(0.66, -.7, 0));
	font << "Impulse X: ";

	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(0, 1, .6));
	font.setPosition(XMFLOAT3(0.84, -.7, 0));
	font << std::to_string(impulseUI.x);


	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(21.0, 106.0, 242.0));
	font.setPosition(XMFLOAT3(0.66, -.8, 0));
	font << "Impulse Z: ";

	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(0, 1, .6));
	font.setPosition(XMFLOAT3(0.84, -.8, 0));
	font << std::to_string(impulseUI.y);

	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(21.0, 106.0, 242.0));
	font.setPosition(XMFLOAT3(0.66, -.9, 0));
	font << "Impulse z: ";

	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(0, 1, .6));
	font.setPosition(XMFLOAT3(0.84, -.9, 0));
	font << std::to_string(impulseUI.z);

	//player lives
	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(21.0, 106.0, 242.0));
	font.setPosition(XMFLOAT3(-0.99, .99,0));
	font << "Player lives: ";

	font.setScaling(XMFLOAT3(1, 1, 1));
	font.setColor(XMFLOAT3(0, 1, .6));
	font.setPosition(XMFLOAT3(-0.8, .99, 0));
	font << std::to_string(playerLives);

	//-----------------------------------------------------------------------------------
	//Play Area Warning
	//-----------------------------------------------------------------------------------

	

	if (abs(cam.position.x) > playerField - 200 || abs(cam.position.y) > playerField - 200 || abs(cam.position.z) > playerField -200) {//checking if a player has gone too far from boundrys
		font.setScaling(XMFLOAT3(1.3, 1.3, 0.0));
		font.setColor(XMFLOAT3(1, 0, 0));
		font.setPosition(XMFLOAT3(-.5, -.5, 0.0));
		font << "WARNING! Approaching Enemy Controled space";
		font.setScaling(XMFLOAT3(1, 1, 1));
		font.setPosition(XMFLOAT3(-.2, -.6, 0.0));
		font << "DNC line in: ";
		font.setPosition(XMFLOAT3(0, -.6, 0.0));

		font << std::to_string(abs(cam.position.x / 100));
		font.setPosition(XMFLOAT3(0, -.65, 0.0));

		font << std::to_string(abs(cam.position.y / 100));

		font.setPosition(XMFLOAT3(0, -.7, 0.0));

		font << std::to_string(abs(cam.position.z / 100));

		if (abs(cam.position.x) > playerField || abs(cam.position.y) > playerField || abs(cam.position.z) > playerField)
			PostQuitMessage(1);

	
	}



	///-----------------------------------------------------------------------------------
	//Explosions
	//-----------------------------------------------------------------------------------
	view = cam.get_matrix(&g_View);
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);
	explosionhandler.render(&view, &g_Projection, elapsed);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);

	
	

	//-----------------------------------------------------------------------------------
	//Instance Rendering
	//-----------------------------------------------------------------------------------
	S = XMMatrixScaling(50, 50, 50);
	R = XMMatrixRotationX(XM_PIDIV2);
	T = XMMatrixTranslation(0, -5, 0);
	M = S*R*T;
	constantbuffer.World = XMMatrixTranspose(M);

	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_screen, &stride, &offset);
	//g_pImmediateContext->Draw(6, 0);

	static float rotation = 0;
	rotation += 0.0000003*elapsed;
	constantbuffer.info.z = rotation;
	constantbuffer.View = XMMatrixTranspose(view);
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->VSSetShader(g_pInstanceShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->IASetInputLayout(g_pInstanceLayout);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture_asteroid);
	g_pImmediateContext->VSSetShaderResources(0, 1, &g_pTexture_asteroid);
	ID3D11Buffer* vertInstBuffer[2] = { g_pVertexBuffer_3ds_asteroids, NULL };
	UINT strides[2] = { stride, sizeof(XMFLOAT4)* 2 };
	UINT offsets[2] = { 0, 0 };
	vertInstBuffer[1] = g_pInstancebuffer;
	g_pImmediateContext->IASetVertexBuffers(0, 2, vertInstBuffer, strides, offsets);
	g_pImmediateContext->DrawInstanced(model_vertex_anz_asteroids, 1000, 0, 0);
	

	



	//-----------------------------------------------------------------------------------
	//Collision detection
	//-----------------------------------------------------------------------------------

	//mines 
	for (int ii = 0; ii < StationaryMines.size(); ii++) {
		float dx = -cam.position.x - StationaryMines[ii]->pos.x;
		float dy = -cam.position.y - StationaryMines[ii]->pos.y;
		float dz = -cam.position.z - StationaryMines[ii]->pos.z;
		float c = sqrt((dx*dx) + (dz*dz) + (dy*dy));

		if (c < 50) {
			//change color
			S = XMMatrixScaling(ms, ms, ms); //NEED TO MAKE A GLOBAL VAR
			if (c < 20)
			{
				explosionhandler.new_explosion(XMFLOAT3(StationaryMines[ii]->pos.x, StationaryMines[ii]->pos.y, StationaryMines[ii]->pos.z), XMFLOAT3(0, 0, 0), 0, 8.0f); //end game
				StationaryMines.erase(StationaryMines.begin() + ii);
				//playerDeath();
			}
		}

	}
	//bullets lossions //USED TO DETCH ENEMY HIT, MINE HIT and weather to delete.
	
	//Temp solution check for distance later
	if (bullets.size() > 15) {
	
		bullets.erase(bullets.begin() + 10);
	
	}


	//One ups
	for (int ii = 0; ii < oneUps.size(); ii++) {
		float dx = -cam.position.x - oneUps[ii]->x;
		float dy = -cam.position.y - oneUps[ii]->y;
		float dz = -cam.position.z - oneUps[ii]->z;
		float c = sqrt((dx*dx) + (dz*dz) + (dy*dy));

		if (c < 50) {
			
			oneUps.erase(oneUps.begin() + ii);
			playerLives++;

		}

	}



	//astroid
	for (int i = 0; i < ASTEROIDCOUNT * 2; i += 2) {
		float dx = -cam.position.x - asteroid_pos[i].x;
		float dy = -cam.position.y - asteroid_pos[i].y;
		float dz = -cam.position.z - asteroid_pos[i].z;
		float c = sqrt((dx*dx) + (dz*dz) + (dy*dy));
		if (c < 20) {
			playerDeath(); // need to remove astroid from the playing field. 
		}
	}

	//
	// Present our back buffer to our front buffer
	//
	g_pSwapChain->Present(0, 0);

	}
//############################################################################################################
void Render_to_screen(long elapsed)
	{
	//and now render it on the screen:
	ConstantBuffer constantbuffer;
	XMMATRIX view = cam.get_matrix(&g_View);
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	constantbuffer.LightView= view;
	constantbuffer.View = XMMatrixTranspose(view);
	constantbuffer.Projection = XMMatrixTranspose(g_Projection);
	constantbuffer.CameraPos = XMFLOAT4(cam.position.x, cam.position.y, cam.position.z, 1);

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	// Clear the back buffer
	float ClearColor2[4] = { 0.0f, 1.0f, 0.0f, 1.0f }; // red, green, blue, alpha

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor2);
	// Clear the depth buffer to 1.0 (max depth)
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);




	constantbuffer.World = XMMatrixIdentity();
	g_pImmediateContext->UpdateSubresource(g_pCBuffer, 0, NULL, &constantbuffer, 0, 0);


	// Render screen


	g_pImmediateContext->VSSetShader(g_pVertexShader_screen, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader_screen, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBuffer);

	ID3D11ShaderResourceView*           texture = RenderToTexture.GetShaderResourceView();// THE MAGIC
	//texture = DepthLight.GetShaderResourceView();// THE MAGIC


	g_pImmediateContext->GenerateMips(texture);
	//texture = g_pTextureRV;
	g_pImmediateContext->PSSetShaderResources(0, 1, &texture);
	g_pImmediateContext->VSSetShaderResources(0, 1, &texture);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_screen, &stride, &offset);
	g_pImmediateContext->PSSetSamplers(0, 1, &SamplerScreen);
	g_pImmediateContext->VSSetSamplers(0, 1, &SamplerScreen);

	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);
	g_pImmediateContext->Draw(6, 0);

	g_pSwapChain->Present(0, 0);
	}


//############################################################################################################
void Render()
{
static StopWatchMicro_ stopwatch;
long elapsed = stopwatch.elapse_micro();
stopwatch.start();//restart

cam.animation(elapsed);
Render_from_light_source(elapsed);
Render_to_texture(elapsed);
Render_to_screen(elapsed);
}

