#include "volumecontroler.h"



int main(int argc, char *argv[])
{
    VolumeController* v = new VolumeController();
    v->connected();

    std::wstring input;
    std::wstring name;
    float volume;
    while(true){
        wcin >> input;
        if(input == L"get"){
            v->handlePacket(std::make_tuple(input, L"", 0.0));
        }
        else if(input == L"set"){
            wcin >> name;
            wcin >> volume;
            v->handlePacket(std::make_tuple(input, name, volume));
        }
    }
}
