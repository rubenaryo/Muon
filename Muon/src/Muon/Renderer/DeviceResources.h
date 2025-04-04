/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Interface definitions for 
IDeviceNotify - Parent of Game, for apps that use deviceresources
DeviceResources - Member of Game, encapsulates the creation and setup of device resources
----------------------------------------------*/
#ifndef DEVICERESOURCES_H
#define DEVICERESOURCES_H

#include "DXCore.h"

namespace Renderer {

// Interface for any device that uses device resources.
// This is because it needs to be able to handle loss of device so it can recreate the necessary device resources
interface IDeviceNotify
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;

protected:
    ~IDeviceNotify() = default;
};

class DeviceResources
{
private:
    enum class DR_OPTIONS : uint8_t
    {
        DR_FLIP_PRESENT  = 1 << 0,
        DR_ALLOW_TEARING = 1 << 1,
        DR_ENABLE_HDR    = 1 << 2,
        DR_ENABLE_MSAA   = 1 << 3
    };
    uint8_t mDeviceOptions;
    inline bool OptionEnabled(DR_OPTIONS option) { return mDeviceOptions & (uint8_t)option; }

public:

    DeviceResources(
        DXGI_FORMAT       backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT       depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
        UINT              backBufferCount   = 2,
        D3D_FEATURE_LEVEL minFeatureLevel   = D3D_FEATURE_LEVEL_10_0,
        uint8_t           flags             = static_cast<uint8_t>(DR_OPTIONS::DR_FLIP_PRESENT)
    ) noexcept;

    virtual ~DeviceResources();

    void CreateDeviceResources();
    void CreateWindowSizeDependentResources();
    void SetWindow(HWND, int width, int height);
    bool WindowSizeChanged(int width, int height);
    void HandleDeviceLost();
    void RegisterDeviceNotify(IDeviceNotify* device) { mpDeviceNotify = device; }
    void Present();
    void Clear(const FLOAT*);

    // Member field accessors
    ID3D11Device*            GetDevice()            const { return mpDevice;             }
    ID3D11DeviceContext*     GetContext()           const { return mpContext;            }
    IDXGISwapChain*          GetSwapChain()         const { return mpSwapChain;          }
    D3D_FEATURE_LEVEL        GetDeviceFeatureLevel()const { return mFeatureLevel;        }
    ID3D11Texture2D*         GetRenderTarget()      const { return mpRenderTarget;       }
    ID3D11Texture2D*         GetDepthStencil()      const { return mpDepthStencilTex;    }
    ID3D11RenderTargetView*  GetRenderTargetView()  const { return mpRenderTargetView;   }
    ID3D11DepthStencilView*  GetDepthStencilView()  const { return mpDepthStencilView;   }
    DXGI_FORMAT              GetBackBufferFormat()  const { return mBackBufferFormat;    }
    DXGI_FORMAT              GetDepthBufferFormat() const { return mDepthBufferFormat;   }
    D3D11_VIEWPORT           GetScreenViewport()    const { return mViewportInfo;        }
    UINT                     GetBackBufferCount()   const { return mBackBufferCount;     }
    DXGI_COLOR_SPACE_TYPE    GetColorSpace()        const { return mColorSpaceType;      }
    unsigned int             GetDeviceOptions()     const { return mDeviceOptions;       }
    RECT                     GetOutputSize()        const { return mOutputSize;          }
    HWND                     GetWindow()            const { return mWindow;              }

    void UpdateTitleBar(uint32_t fps, uint32_t frameCount);

private:
    void CreateFactory();
    void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
    void UpdateColorSpace();
    void ReleaseAllComAndDumpLiveObjects();
    inline void ReportLiveDeviceObjects_d();


    // Direct3D Objects (COM RefCounted)
    IDXGIFactory*                mpDXGIFactory;
    ID3D11Device*                mpDevice;
    ID3D11DeviceContext*         mpContext = nullptr;
    IDXGISwapChain*              mpSwapChain;
    UINT                         mPresentFlags;

    ID3D11Texture2D*             mpRenderTarget;
    ID3D11Texture2D*             mpDepthStencilTex;
    ID3D11RenderTargetView*      mpRenderTargetView;
    ID3D11DepthStencilView*      mpDepthStencilView;

    D3D11_VIEWPORT               mViewportInfo;

    // Necessary members for MSAA (COM RefCounted)
    ID3D11Texture2D*             mpMSAARenderTarget;
    ID3D11RenderTargetView*      mpMSAARenderTargetView;
    ID3D11DepthStencilView*      mpMSAADepthStencilView;
    unsigned int                 mMSAASampleCount;  // 4X, 8X, etc
                                        
    // Direct3D Properties              
    DXGI_FORMAT                  mBackBufferFormat;
    DXGI_FORMAT                  mDepthBufferFormat;
    UINT                         mBackBufferCount;
    D3D_FEATURE_LEVEL            mMinFeatureLevel;


    // Cached Device Properties
    HWND                         mWindow;
    D3D_FEATURE_LEVEL            mFeatureLevel;
    RECT                         mOutputSize;
                                        
    // HDR Support                      
    DXGI_COLOR_SPACE_TYPE        mColorSpaceType;
                                        
    // Pointer to DeviceNotify Interface
    IDeviceNotify* mpDeviceNotify;    
    
#if defined(MN_DEBUG)
    ID3DUserDefinedAnnotation*   mpAnnotation;
    ID3D11Debug*                 mpDebugInterface;
#endif

};

}
#endif