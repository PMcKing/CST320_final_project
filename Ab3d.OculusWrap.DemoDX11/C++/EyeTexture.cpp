//.h file code:

#include <vector>

using namespace SharpDX;
using namespace SharpDX::Direct3D11;

namespace Ab3d
{
	namespace OculusWrap
	{
		namespace DemoDX11
		{
			/// <summary>
			/// Contains all the fields used by each eye.
			/// </summary>
			class EyeTexture
			{
			public:
				Texture2DDescription *Texture2DDescription;
				TextureSwapChain *SwapTextureSet;
				std::vector<Texture2D*> Textures;
				std::vector<RenderTargetView*> RenderTargetViews;
				Texture2DDescription *DepthBufferDescription;
				Texture2D *DepthBuffer;
				Viewport *Viewport;
				DepthStencilView *DepthStencilView;
				FovPort *FieldOfView;
				Sizei *TextureSize;
				Recti *ViewportSize;
				EyeRenderDesc *RenderDescription;
				Vector3f *HmdToEyeViewOffset;

		//		#region IDisposable Members
				/// <summary>
				/// Dispose contained fields.
				/// </summary>
				~EyeTexture();
		//		#endregion
			};
		}
	}
}

//.cpp file code:

using namespace SharpDX;
using namespace SharpDX::Direct3D11;
namespace Ab3d
{
	namespace OculusWrap
	{
		namespace DemoDX11
		{

			EyeTexture::~EyeTexture()
			{
						delete Texture2DDescription;
						delete DepthBufferDescription;
						delete DepthBuffer;
						delete Viewport;
						delete DepthStencilView;
						delete FieldOfView;
						delete TextureSize;
						delete ViewportSize;
						delete RenderDescription;
						delete HmdToEyeViewOffset;
				if (SwapTextureSet != nullptr)
				{
					delete SwapTextureSet;
					SwapTextureSet = nullptr;
				}

				if (Textures.size() > 0)
				{
					for (auto texture : Textures)
					{
						delete texture;
					}

					Textures.clear();
				}

				if (RenderTargetViews.size() > 0)
				{
					for (auto renderTargetView : RenderTargetViews)
					{
						delete renderTargetView;
					}

					RenderTargetViews.clear();
				}

				if (DepthBuffer != nullptr)
				{
					delete DepthBuffer;
					DepthBuffer = nullptr;
				}

				if (DepthStencilView != nullptr)
				{
					delete DepthStencilView;
					DepthStencilView = nullptr;
				}
			}
		}
	}
}
