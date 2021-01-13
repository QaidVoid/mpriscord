#include <mpris.h>
#include <sdk/discord.h>
#include <thread>
#include <iostream>
#include <csignal>

const long int CLIENT = 797378815995150336;

struct DiscordState
{
    std::unique_ptr<discord::Core> core;
};

namespace
{
    volatile bool interrupted{false};
}

void SetActivity(DiscordState &state)
{
    auto mpris = new Mpris;
    auto metadata = mpris->GetMetadata();

    if (metadata != nullptr)
    {
        const char *player = metadata->player.c_str();
        const char *title = metadata->title.c_str();
        std::string artist = std::string("by ").append(metadata->artist);
        std::string album = std::string("\nAlbum: ").append(metadata->album);

        discord::Activity activity{};
        activity.SetType(discord::ActivityType::Listening);
        activity.SetDetails(title);
        activity.SetState(artist.c_str());
        activity.GetAssets().SetLargeImage(player);
        state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
                      << " updating activity!\n";
        });
    } else {
        state.core->ActivityManager().ClearActivity([](discord::Result result) {
            std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
                      << " clearing activity!\n";
        });
    }
}

int main()
{
    DiscordState state{};
    discord::Core *core{};
    auto result = discord::Core::Create(CLIENT, DiscordCreateFlags_Default, &core);
    state.core.reset(core);
    if (!state.core)
    {
        std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
                  << ")\n";
        std::exit(1);
    }

    state.core->SetLogHook(
        discord::LogLevel::Debug, [](discord::LogLevel level, const char *message) {
            std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
        });

    std::signal(SIGINT, [](int) { interrupted = true; });
    do
    {
        SetActivity(state);
        state.core->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } while (!interrupted);
}