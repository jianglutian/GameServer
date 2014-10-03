
#ifndef MCACHED_H_
#define MCACHED_H_

#include <vector>
#include <libmemcached/memcached.h>

namespace buf
{

#define MEMCACHED_PORT 11211

class MCached
{
public:
    MCached();
    ~MCached();

    // XXX: host:port,host:port,host:port,...
    bool init(const std::string& hosts);
    bool init(const char* hosts);

    bool pushHost(const std::string& hosts);
    bool pushHost(const char* hosts);

    const char* get(const char* key, char* value, size_t size);
    const char* get(const char* key, size_t key_size, char* value, size_t size);

    bool set(const char* key, const char* value, int timeout = 0);
    bool set(const char* key, size_t key_size, const char* value, size_t size, int timeout = 0);

    const std::vector<std::string>& getHosts() const { return _hosts; }

protected:
    bool addHosts(const char* hosts);

private:
    std::vector<std::string> _hosts;
    memcached_st* _memsvr;
};

} // namespace buf

#endif // MCACHED_H_

/* vim: set ai si nu sm smd hls is ts=4 sm=4 bs=indent,eol,start */

