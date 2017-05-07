//.h file code:

#include <stdexcept>
#include "tangible_event.h"


namespace Ab3d
{
	namespace OculusWrap
	{
		namespace DemoDX11
		{
			/// <summary>
			/// Wrapper for the MirrorTexture type.
			/// </summary>
			class MirrorTexture
			{
			private:
				void* privateMirrorTexturePtr;
				bool privateIsDisposed = false;

				void* _sessionPtr;

				OvrWrap *_ovr;

				/// <summary>
				/// Pointer to unmanaged MirrorTexture.
				/// </summary>
					public:
						IntPtr getMirrorTexturePtr() const;
						void setMirrorTexturePtr(IntPtr value);

				/// <summary>
				/// Describes if the object has been disposed.
				/// </summary>
						bool getIsDisposed() const;
						void setIsDisposed(const bool &value);

				/// <summary>
				/// Notifies subscribers when this object has been disposed.
				/// </summary>
				TangibleEvent<System.Action<MirrorTexture>> *Disposed = new TangibleEvent<System.Action<MirrorTexture>>();

				/// <summary>
				/// Creates a new MirrorTexture.
				/// </summary>
				/// <param name="ovr">Interface to Oculus runtime methods.</param>
				/// <param name="sessionPtr">IntPtr of the OVR </param>
				/// <param name="mirrorTexturePtr">Unmanaged mirror texture.</param>
				MirrorTexture(OvrWrap *ovr, void* sessionPtr, void* mirrorTexturePtr);

		//		#region IDisposable Members
				/// <summary>
				/// Clean up the allocated HMD.
				/// </summary>
				~MirrorTexture();

				/// <summary>
				/// Dispose pattern implementation of dispose method.
				/// </summary>
				/// <param name="disposing">True if disposing, false if finalizing.</param>
			private:
				void Dispose(bool disposing);
		//		#endregion

		//		#region Public methods
				/// <summary>
				/// Get a the underlying buffer as any compatible COM interface (similar to QueryInterface) 
				/// </summary>
				/// <param name="iid">Specifies the interface ID of the interface pointer to query the buffer for.</param>
				/// <param name="buffer">Returns the COM interface pointer retrieved.</param>
				/// <returns>
				/// Returns a Result indicating success or failure. In the case of failure, use 
				/// Wrap.GetLastError to get more information.
				/// </returns>
			public:
				Result *GetBufferDX(Guid iid, void* &buffer);

				/// <summary>
				/// Get a the underlying buffer as a GL texture name
				/// </summary>
				/// <param name="textureId">Specifies the GL texture object name associated with the mirror texture</param>
				/// <returns>
				/// Returns an OVRTypes.Result indicating success or failure. In the case of failure, use 
				/// Wrap.GetLastError to get more information.
				/// </returns>
				Result *GetBufferGL(unsigned int &textureId);

		//		#endregion
			};
		}
	}
}

//tangible_event.h:

//----------------------------------------------------------------------------------------
//	Copyright © 2004 - 2017 Tangible Software Solutions Inc.
//	This class can be used by anyone provided that the copyright notice remains intact.
//
//	This class is used to convert C# events to C++.
//----------------------------------------------------------------------------------------
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

template<typename T>
class TangibleEvent final
{
private:
	std::unordered_map<std::wstring, T> namedListeners;
public:
	void addListener(const std::wstring &methodName, T namedEventHandlerMethod)
	{
		if (namedListeners.find(methodName) == namedListeners.end())
			namedListeners[methodName] = namedEventHandlerMethod;
	}
	void removeListener(const std::wstring &methodName)
	{
		if (namedListeners.find(methodName) != namedListeners.end())
			namedListeners.erase(methodName);
	}

private:
	std::vector<T> anonymousListeners;
public:
	void addListener(T unnamedEventHandlerMethod)
	{
		anonymousListeners.push_back(unnamedEventHandlerMethod);
	}

	std::vector<T> listeners()
	{
		std::vector<T> allListeners;
		for (auto listener : namedListeners)
		{
			allListeners.push_back(listener.second);
		}
		allListeners.insert(allListeners.end(), anonymousListeners.begin(), anonymousListeners.end());
		return allListeners;
	}
};

//.cpp file code:

namespace Ab3d
{
	namespace OculusWrap
	{
		namespace DemoDX11
		{

			IntPtr MirrorTexture::getMirrorTexturePtr() const
			{
				return privateMirrorTexturePtr;
			}

			void MirrorTexture::setMirrorTexturePtr(const IntPtr &value)
			{
				privateMirrorTexturePtr = value;
			}

			bool MirrorTexture::getIsDisposed() const
			{
				return privateIsDisposed;
			}

			void MirrorTexture::setIsDisposed(const bool &value)
			{
				privateIsDisposed = value;
			}

			MirrorTexture::MirrorTexture(OvrWrap *ovr, void* sessionPtr, void* mirrorTexturePtr)
			{
				if (ovr == nullptr)
				{
					throw std::invalid_argument("ovr");
				}

				if (sessionPtr == void*::Zero)
				{
					throw std::invalid_argument("sessionPtr");
				}

				if (mirrorTexturePtr == void*::Zero)
				{
					throw std::invalid_argument("mirrorTexturePtr");
				}

				_ovr = ovr;
				_sessionPtr = sessionPtr;
				setMirrorTexturePtr(mirrorTexturePtr);
			}

			MirrorTexture::~MirrorTexture()
			{
						delete _ovr;
				this->Dispose(true);
//C# TO C++ CONVERTER WARNING: There is no garbage collector in native C++:
//				GC::SuppressFinalize(this);
			}

			void MirrorTexture::Dispose(bool disposing)
			{
				if (getIsDisposed())
				{
					return;
				}

				if (getMirrorTexturePtr() != void*::Zero)
				{
					_ovr->DestroyMirrorTexture(_sessionPtr, getMirrorTexturePtr());
					setMirrorTexturePtr(void*::Zero);

					// Notify subscribers that this object has been disposed.
					if (Disposed != nullptr)
					{
						for (auto listener : Disposed->listeners())
						{
							listener(this);
						}
					}
				}

//C# TO C++ CONVERTER WARNING: There is no garbage collector in native C++:
//				GC::SuppressFinalize(this);

				setIsDisposed(true);
			}

			Result *MirrorTexture::GetBufferDX(Guid iid, void* &buffer)
			{
				return _ovr->GetMirrorTextureBufferDX(_sessionPtr, getMirrorTexturePtr(), iid, buffer);
			}

			Result *MirrorTexture::GetBufferGL(unsigned int &textureId)
			{
				return _ovr->GetMirrorTextureBufferGL(_sessionPtr, getMirrorTexturePtr(), textureId);
			}
		}
	}
}
