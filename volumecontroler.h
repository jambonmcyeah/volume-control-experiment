#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H
#include <stdio.h>
#include <string>
#include <iostream>
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
        CMMNotificationClient(VolumeController *x);

        ~CMMNotificationClient();

        // IUnknown methods -- AddRef, Release, and QueryInterface

        ULONG STDMETHODCALLTYPE AddRef();

        ULONG STDMETHODCALLTYPE Release();

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface);

        // Callback methods for device-event notifications.

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);

        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);

        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);

        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);

        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

      private:
        VolumeController *enclosing;
    };
    class CAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback
    {
        LONG _cRef;

      public:
        CAudioEndpointVolumeCallback(VolumeController *x, std::string sinkName);
        ~CAudioEndpointVolumeCallback();

        // IUnknown methods -- AddRef, Release, and QueryInterface

        ULONG STDMETHODCALLTYPE AddRef();

        ULONG STDMETHODCALLTYPE Release();

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface);

        // Callback method for endpoint-volume-change notifications.

        HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

      private:
        VolumeController *enclosing;
        std::string name;
    };
    IMMDeviceEnumerator *deviceEnumerator;
    IMMNotificationClient *deviceCallback;
    std::map<std::string, std::tuple<IAudioEndpointVolume *, IAudioEndpointVolumeCallback *>> sinkList;

  public:
    VolumeController();
    ~VolumeController();
    void sendSinkList();
    void connected();
};

#endif // VOLUMECONTROL_H
