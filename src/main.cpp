#include <discord.h>
#include <csignal>

namespace
{
    volatile bool interrupted{false};
}

int main()
{
    DiscordState state{};

    std::thread t(&DiscordState::SetMetadata, &state);
    t.detach();

    std::signal(SIGINT, [](int) { interrupted = true; });
    do
    {
        if (state.core != nullptr)
            state.core->RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } while (!interrupted);
}