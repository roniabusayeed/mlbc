#include <stdexcept>
#include "fileWatcher.h"


Watcher::Watcher(const std::string& directory, Callback callback, bool recursive) {
    m_file_watcher = std::make_unique<efsw::FileWatcher>();
    m_file_watch_listener = std::make_unique<FileWatchListenerImpl>(std::move(callback));
    m_watch_id = m_file_watcher->addWatch(directory, m_file_watch_listener.get(), recursive);
    
    // error checking
    switch (m_watch_id)
    {
    case efsw::Errors::Error::FileNotFound:
    case efsw::Errors::Error::FileRepeated:
    case efsw::Errors::Error::FileOutOfScope:
    case efsw::Errors::Error::FileNotReadable:
    case efsw::Errors::Error::FileRemote:
    case efsw::Errors::Error::WatcherFailed:
    case efsw::Errors::Error::Unspecified:
        throw std::runtime_error("couldn't watch directory: " + directory);
    }

    m_file_watcher->watch();
}

Watcher::~Watcher() {    
    m_file_watcher->removeWatch(m_watch_id);
}

Watcher::FileWatchListenerImpl::FileWatchListenerImpl(Callback cb) : callback(std::move(cb)) {}

void Watcher::FileWatchListenerImpl::handleFileAction(efsw::WatchID watchid, const std::string& dir,
                                                      const std::string& filename, efsw::Action action,
                                                      std::string old_filename) {
    if (callback) {
        callback(dir, filename, action, old_filename);
    }
}
