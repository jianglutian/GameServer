
#include "Path_POSIX.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#if !defined(_VXWORKS)
#include <pwd.h>
#endif
#include <climits>

#ifndef PATH_MAX
#define PATH_MAX 4096 // XXX: 
#endif

namespace buf
{

std::string PathImpl::currentImpl()
{
	std::string path;
	char cwd[PATH_MAX];

	if (getcwd(cwd, sizeof(cwd)))
		path = cwd;
	else
        return "";

	std::string::size_type n = path.size();
	if (n > 0 && path[n-1] != '/') path.append("/");

	return path;
}

std::string PathImpl::homeImpl()
{
#if defined(_VXWORKS)
	return "/";
#else
	std::string path;
	struct passwd* pwd = getpwuid(getuid());
	if (pwd)
		path = pwd->pw_dir;
	else {
		pwd = getpwuid(geteuid());
		if (pwd)
			path = pwd->pw_dir;
		else
            path = getenv("HOME");
	}

	std::string::size_type n = path.size();
	if (n > 0 && path[n-1] != '/') path.append("/");

	return path;
#endif
}

std::string PathImpl::tempImpl()
{
	std::string path;
	char* tmp = getenv("TMPDIR");
	if (tmp) {
		path = tmp;
		std::string::size_type n = path.size();
		if (n > 0 && path[n - 1] != '/') path.append("/");
	} else {
		path = "/tmp/";
	}
	return path;
}

std::string PathImpl::nullImpl()
{
	return "/dev/null";
}

std::string PathImpl::expandImpl(const std::string& path)
{
	std::string result;
	std::string::const_iterator it  = path.begin();
	std::string::const_iterator end = path.end();
	if (it != end && *it == '~') {
		++it;
		if (it != end && *it == '/') {
			result += homeImpl(); ++it;
		} else {
            result += '~';
        }
	}

	while (it != end) {
		if (*it == '$') {
			std::string var;
			++it;
			if (it != end && *it == '{') {
				++it;
				while (it != end && *it != '}')
                    var += *it++;
				if (it != end)
                    ++it;
			} else {
				while (it != end && (isalnum(*it) || *it == '_'))
                    var += *it++;
			}
			char* val = getenv(var.c_str());
			if (val) result += val;
		} else {
            result += *it++;
        }
	}
	return result;
}

void PathImpl::listRootsImpl(std::vector<std::string>& roots)
{
	roots.clear();
	roots.push_back("/");
}

} // namespace buf

/* vim: set ai si nu sm smd hls is ts=4 sm=4 bs=indent,eol,start */

