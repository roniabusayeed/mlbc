#pragma once

#include <efsw/efsw.hpp>
#include <functional>
#include <memory>
#include <string>


class Watcher {
public:
    using Callback = std::function<void(const std::string&, const std::string&, efsw::Action, const std::string&)>;

    Watcher(const std::string& directory, Callback callback, bool recursive = false);
    ~Watcher();

private:
    class FileWatchListenerImpl : public efsw::FileWatchListener {
    public:
        explicit FileWatchListenerImpl(Callback cb);

        void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                              const std::string& filename, efsw::Action action,
                              std::string old_filename) override;

    private:
        Callback callback;
    };

    std::unique_ptr<efsw::FileWatcher> m_file_watcher;
    std::unique_ptr<FileWatchListenerImpl> m_file_watch_listener;
    efsw::WatchID m_watch_id;
};
