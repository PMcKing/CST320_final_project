//.h file code:

#include <stdexcept>
#include "exceptionhelper.h"
#include "tangible_event.h"


namespace Ab3d
{
	namespace OculusWrap
	{
		namespace DemoDX11
		{
			/// <summary>
			/// Wrapper for the TextureSwapChain type.
			/// </summary>
			class TextureSwapChain
			{
			private:
				void* privateTextureSwapChainPtr;
				bool privateIsDisposed = false;

				void* _sessionPtr;

				OvrWrap *_ovr;

				/// <summary>
				/// Pointer to unmanaged SwapTextureSet.
				/// </summary>
					public:
						IntPtr getTextureSwapChainPtr() const;
						void setTextureSwapChainPtr(IntPtr value);

				/// <summary>
				/// Describes if the object has been disposed.
				/// </summary>
						bool getIsDisposed() const;
						void setIsDisposed(const bool &value);

				/// <summary>
				/// Notifies subscribers when this object has been disposed.
				/// </summary>
				TangibleEvent<System.Action<TextureSwapChain>> *Disposed = new TangibleEvent<System.Action<TextureSwapChain>>();

				/// <summary>
				/// Creates a new TextureSwapChain.
				/// </summary>
				/// <param name="ovr">Interface to Oculus runtime methods.</param>
				/// <param name="sessionPtr">Session of the Hmd owning this texture swap chain.</param>
				/// <param name="textureSwapChainPtr">Unmanaged texture swap chain.</param>
				TextureSwapChain(OvrWrap *ovr, void* sessionPtr, void* textureSwapChainPtr);

		//		#region IDisposable Members
				/// <summary>
				/// Clean up the allocated HMD.
				/// </summary>
				~TextureSwapChain();

				/// <summary>
				/// Dispose pattern implementation of dispose method.
				/// </summary>
				/// <param name="disposing">True if disposing, false if finalizing.</param>
			private:
				void Dispose(bool disposing);
		//		#endregion

		//		#region Public methods
				/// <summary>
				/// Gets the number of buffers in the TextureSwapChain.
				/// </summary>
				/// <param name="length">Returns the number of buffers in the specified chain.</param>
				/// <returns>Returns an ovrResult for which the return code is negative upon error. </returns>
			public:
				Result *GetLength(int &length);

				/// <summary>
				/// Gets the current index in the TextureSwapChain.
				/// </summary>
				/// <param name="index">Returns the current (free) index in specified chain.</param>
				/// <returns>Returns an ovrResult for which the return code is negative upon error. </returns>
				Result *GetCurrentIndex(int &index);

				/// <summary>
				/// Gets the description of the buffers in the TextureSwapChain
				/// </summary>
				/// <param name="textureSwapChainDescription">Returns the description of the specified chain.</param>
				/// <returns>Returns an ovrResult for which the return code is negative upon error. </returns>
				Result *GetDescription(TextureSwapChainDesc *&textureSwapChainDescription);

				/// <summary>
				/// Commits any pending changes to a TextureSwapChain, and advances its current index
				/// </summary>
				/// <returns>
				/// Returns an ovrResult for which the return code is negative upon error.
				/// Failures include but aren't limited to:
				///   - Result.TextureSwapChainFull: ovr_CommitTextureSwapChain was called too many times on a texture swapchain without calling submit to use the chain.
				/// </returns>
				Result *Commit();

				/// <summary>
				/// Get a specific buffer within the chain as any compatible COM interface (similar to QueryInterface)
				/// </summary>
				/// <param name="index">
				/// Specifies the index within the chain to retrieve. Must be between 0 and length (see GetTextureSwapChainLength),
				/// or may pass -1 to get the buffer at the CurrentIndex location. (Saving a call to GetTextureSwapChainCurrentIndex).
				/// </param>
				/// <param name="iid">Specifies the interface ID of the interface pointer to query the buffer for.</param>
				/// <param name="buffer">Returns the COM interface pointer retrieved.</param>
				/// <returns>
				/// Returns an ovrResult indicating success or failure. In the case of failure, use 
				/// Wrap.GetLastError to get more information.
				/// </returns>
				Result *GetBufferDX(int index, Guid iid, void* &buffer);

				/// <summary>
				/// Get a specific buffer within the chain as a GL texture name
				/// </summary>
				/// <param name="index">
				/// Specifies the index within the chain to retrieve. Must be between 0 and length (see GetTextureSwapChainLength)
				/// or may pass -1 to get the buffer at the CurrentIndex location. (Saving a call to GetTextureSwapChainCurrentIndex)
				/// </param>
				/// <param name="textureId">Returns the GL texture object name associated with the specific index requested</param>
				/// <returns>
				/// Returns an Result indicating success or failure. In the case of failure, use 
				/// Wrap.GetLastError to get more information.
				/// </returns>
				Result *GetBufferGL(int index, unsigned int &textureId);

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

//exceptionhelper.h:

#include <stdexcept>

class ObjectDisposedException : public std::exception
{
private:
    std::string msg;

public:
    ObjectDisposedException(const std::string& message = "") : msg(message)
    {
    }

    const char * what() const throw()
    {
        return msg.c_str();
    }
};

//.cpp file code:

namespace Ab3d
{
	namespace OculusWrap
	{
		namespace DemoDX11
		{

			IntPtr TextureSwapChain::getTextureSwapChainPtr() const
			{
				return privateTextureSwapChainPtr;
			}

			void TextureSwapChain::setTextureSwapChainPtr(const IntPtr &value)
			{
				privateTextureSwapChainPtr = value;
			}

			bool TextureSwapChain::getIsDisposed() const
			{
				return privateIsDisposed;
			}

			void TextureSwapChain::setIsDisposed(const bool &value)
			{
				privateIsDisposed = value;
			}

			TextureSwapChain::TextureSwapChain(OvrWrap *ovr, void* sessionPtr, void* textureSwapChainPtr)
			{
				if (ovr == nullptr)
				{
					throw std::invalid_argument("ovr");
				}

				if (sessionPtr == void*::Zero)
				{
					throw std::invalid_argument("sessionPtr");
				}

				if (textureSwapChainPtr == void*::Zero)
				{
					throw std::invalid_argument("textureSwapChainPtr");
				}

				_ovr = ovr;
				_sessionPtr = sessionPtr;
				setTextureSwapChainPtr(textureSwapChainPtr);
			}

			TextureSwapChain::~TextureSwapChain()
			{
						delete _ovr;
				this->Dispose(true);
//C# TO C++ CONVERTER WARNING: There is no garbage collector in native C++:
//				GC::SuppressFinalize(this);
			}

			void TextureSwapChain::Dispose(bool disposing)
			{
				if (getIsDisposed())
				{
					return;
				}

				if (getTextureSwapChainPtr() != void*::Zero)
				{
					_ovr->DestroyTextureSwapChain(_sessionPtr, getTextureSwapChainPtr());
					setTextureSwapChainPtr(void*::Zero);

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

			Result *TextureSwapChain::GetLength(int &length)
			{
				if (getIsDisposed())
				{
					throw ObjectDisposedException("TextureSwapChain");
				}

				return _ovr->GetTextureSwapChainLength(_sessionPtr, getTextureSwapChainPtr(), length);
			}

			Result *TextureSwapChain::GetCurrentIndex(int &index)
			{
				if (getIsDisposed())
				{
					throw ObjectDisposedException("TextureSwapChain");
				}

				return _ovr->GetTextureSwapChainCurrentIndex(_sessionPtr, getTextureSwapChainPtr(), index);
			}

			Result *TextureSwapChain::GetDescription(TextureSwapChainDesc *&textureSwapChainDescription)
			{
				if (getIsDisposed())
				{
					throw ObjectDisposedException("TextureSwapChain");
				}

				TextureSwapChainDesc *textureSwapChainDesc = new TextureSwapChainDesc();

				Result *result = _ovr->GetTextureSwapChainDesc(_sessionPtr, getTextureSwapChainPtr(), textureSwapChainDesc);
				textureSwapChainDescription = textureSwapChainDesc;

				return result;
			}

			Result *TextureSwapChain::Commit()
			{
				if (getIsDisposed())
				{
					throw ObjectDisposedException("TextureSwapChain");
				}

				return _ovr->CommitTextureSwapChain(_sessionPtr, getTextureSwapChainPtr());
			}

			Result *TextureSwapChain::GetBufferDX(int index, Guid iid, void* &buffer)
			{
				return _ovr->GetTextureSwapChainBufferDX(_sessionPtr, getTextureSwapChainPtr(), index, iid, buffer);
			}

			Result *TextureSwapChain::GetBufferGL(int index, unsigned int &textureId)
			{
				return _ovr->GetTextureSwapChainBufferGL(_sessionPtr, getTextureSwapChainPtr(), index, textureId);
			}
		}
	}
}
