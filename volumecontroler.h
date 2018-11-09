#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>

#include <Windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

#pragma comment(lib, "ole32")

using namespace std;

class VolumeController
{
  private:
    class CMMNotificationClient : public IMMNotificationClient
    {
        LONG _cRef;

      public:
        CMMNotificationClient(VolumeController& x);

        ~CMMNotificationClient();

        // IUnknown methods -- AddRef, Release, and QueryInterface

        ULONG STDMETHODCALLTYPE AddRef() override;

        ULONG STDMETHODCALLTYPE Release() override;

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override;

        // Callback methods for device-event notifications.

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) override;

        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;

        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;

        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;

        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

      private:
        VolumeController& enclosing;
    };
    class CAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback
    {
        LONG _cRef;

      public:
        CAudioEndpointVolumeCallback(VolumeController& x, std::wstring sinkName);
        ~CAudioEndpointVolumeCallback();

        // IUnknown methods -- AddRef, Release, and QueryInterface

        ULONG STDMETHODCALLTYPE AddRef() override;

        ULONG STDMETHODCALLTYPE Release() override;

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) override;

        // Callback method for endpoint-volume-change notifications.

        HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

      private:
        VolumeController& enclosing;
        std::wstring name;
    };
    IMMDeviceEnumerator *deviceEnumerator;
    IMMNotificationClient *deviceCallback;
    std::map<std::wstring, std::tuple<IAudioEndpointVolume *, CAudioEndpointVolumeCallback *>> sinkList;

  public:
    VolumeController();
    ~VolumeController();
    void sendSinkList();
    void connected();
    void handlePacket(std::tuple<std::wstring, std::wstring, float> packet);
};

#endif // VOLUMECONTROL_H
