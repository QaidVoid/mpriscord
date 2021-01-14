#pragma once
#include <sdk/discord.h>
#include <mpris.h>
#include <thread>

const long int CLIENT = 797378815995150336;
struct DiscordState
{
    std::unique_ptr<discord::Core> core;
    void SetMetadata();
    bool isActive;

private:
    Metadata *metadata;
    void SetActivity();
};
