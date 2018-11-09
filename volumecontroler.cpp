#include "volumecontroler.h"

VolumeController::VolumeController()
{
    CoInitialize(nullptr);
    deviceEnumerator = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&(deviceEnumerator));
    if (hr != S_OK)
    {
        throw hr;
    }
}

VolumeController::~VolumeController()
{
    deviceEnumerator->UnregisterEndpointNotificationCallback(deviceCallback);
    deviceEnumerator->Release();
    deviceEnumerator = nullptr;
}

void VolumeController::sendSinkList()
{
    HRESULT hr;
    if (!sinkList.empty())
    {
        for (auto const &sink : sinkList)
        {
            std::get<0>(sink.second)->UnregisterControlChangeNotify(std::get<1>(sink.second));
            std::get<1>(sink.second)->Release();
            std::get<0>(sink.second)->Release();
        }
        sinkList.clear();
    }
    IMMDeviceCollection *devices = nullptr;
    hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);

    if (hr != S_OK)
    {
        throw hr;
    }
    unsigned int deviceCount;
    devices->GetCount(&deviceCount);
    for (unsigned int i = 0; i < deviceCount; i++)
    {
        IMMDevice *device = nullptr;

        IPropertyStore *deviceProperties = nullptr;
        PROPVARIANT deviceProperty;
        std::wstring name;
        std::wstring desc;
        float volume;
        BOOL muted;

        IAudioEndpointVolume* endpoint = nullptr;
        CAudioEndpointVolumeCallback* callback;
        
        // Get Properties
        devices->Item(i, &device);
        device->OpenPropertyStore(STGM_READ, &deviceProperties);

        deviceProperties->GetValue(PKEY_Device_FriendlyName, &deviceProperty);
        name = deviceProperty.pwszVal;
        //PropVariantClear(&deviceProperty);

        deviceProperties->GetValue(PKEY_Device_DeviceDesc, &deviceProperty);
        desc = deviceProperty.pwszVal;
        //PropVariantClear(&deviceProperty);

        std::cout << "\nDevice Begin"  << std::endl;
        std::wcout << L"Name: " << name << std::endl;
        std::wcout << L"Desc: " << desc << std::endl;

        hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&endpoint);
        if (hr != S_OK)
        {
            std::wcout << L"Warning: Failed to create IAudioEndpointVolume for " << name;
            std::cout << "Code: " << hr;
            continue;
        }
        endpoint->GetMasterVolumeLevelScalar(&volume);
        endpoint->GetMute(&muted);
        std::cout << "Max Volume: 1.0" << std::endl;
        std::cout << "Volume: " << volume << std::endl;
        std::cout << "Muted: " << (bool)muted << std::endl;

        // Register Callback
        callback = new CAudioEndpointVolumeCallback(*this, name);
        sinkList[name] = std::make_tuple(endpoint, callback);
        endpoint->RegisterControlChangeNotify(callback);

        device->Release();
    }
    devices->Release();
}

void VolumeController::connected()
{
    deviceCallback = new CMMNotificationClient(*this);
    deviceEnumerator->RegisterEndpointNotificationCallback(deviceCallback);
    sendSinkList();
}

void VolumeController::handlePacket(std::tuple<std::wstring, std::wstring, float> packet)
{
    if (std::get<0>(packet) == L"get"){
        sendSinkList();
    }
    else if (std::get<0>(packet) == L"set"){
        std::get<0>(sinkList[std::get<1>(packet)])->SetMasterVolumeLevelScalar(std::get<2>(packet), NULL);
    }
}


// CMMNotificationClient

VolumeController::CMMNotificationClient::CMMNotificationClient(VolumeController& x) : enclosing(x), _cRef(1) {}

VolumeController::CMMNotificationClient::~CMMNotificationClient() {}

// IUnknown methods -- AddRef, Release, and QueryInterface

ULONG STDMETHODCALLTYPE VolumeController::CMMNotificationClient::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG STDMETHODCALLTYPE VolumeController::CMMNotificationClient::Release()
{
    ULONG ulRef = InterlockedDecrement(&_cRef);
    if (0 == ulRef)
    {
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE VolumeController::CMMNotificationClient::QueryInterface(REFIID riid, VOID **ppvInterface)
{
    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvInterface = (IUnknown *)this;
    }
    else if (__uuidof(IMMNotificationClient) == riid)
    {
        AddRef();
        *ppvInterface = (IMMNotificationClient *)this;
    }
    else
    {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

// Callback methods for device-event notifications.

HRESULT STDMETHODCALLTYPE VolumeController::CMMNotificationClient::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    if (flow == eRender)
    {
        enclosing.sendSinkList();
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VolumeController::CMMNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    enclosing.sendSinkList();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VolumeController::CMMNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    enclosing.sendSinkList();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VolumeController::CMMNotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    enclosing.sendSinkList();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VolumeController::CMMNotificationClient::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}


// CAudioEndpointVolumeCallback

VolumeController::CAudioEndpointVolumeCallback::CAudioEndpointVolumeCallback(VolumeController& x, std::wstring sinkName) : enclosing(x), name(sinkName), _cRef(1) {}

VolumeController::CAudioEndpointVolumeCallback::~CAudioEndpointVolumeCallback() {}

// IUnknown methods -- AddRef, Release, and QueryInterface

ULONG STDMETHODCALLTYPE VolumeController::CAudioEndpointVolumeCallback::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG STDMETHODCALLTYPE VolumeController::CAudioEndpointVolumeCallback::Release()
{
    ULONG ulRef = InterlockedDecrement(&_cRef);
    if (0 == ulRef)
    {
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE VolumeController::CAudioEndpointVolumeCallback::QueryInterface(REFIID riid, VOID **ppvInterface)
{
    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvInterface = (IUnknown *)this;
    }
    else if (__uuidof(IMMNotificationClient) == riid)
    {
        AddRef();
        *ppvInterface = (IMMNotificationClient *)this;
    }
    else
    {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

// Callback method for endpoint-volume-change notifications.

HRESULT STDMETHODCALLTYPE VolumeController::CAudioEndpointVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    std::cout << "\nDevice Volume Change"  << std::endl;
    std::wcout << L"Name: " << name << std::endl;
    std::cout << "Volume: " << pNotify->fMasterVolume << std::endl;
    std::cout << "Muted: " << pNotify->bMuted << std::endl;

    float volume;
    std::get<0>(enclosing.sinkList[name])->GetMasterVolumeLevelScalar(&volume);
    std::cout << "Volume DB: " << volume << std::endl;
    return S_OK;
}
