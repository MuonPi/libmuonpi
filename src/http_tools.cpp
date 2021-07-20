#include "muonpi/http_tools.h"

namespace muonpi::http {

void fail(beast::error_code ec, const std::string& what)
{
    if (ec == net::ssl::error::stream_truncated) {
        return;
    }

    log::warning() << what << ": " << ec.message();
}

}
