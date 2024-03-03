# Webserv - recreating nginx from scratch in C++
This is the webserv of jmeruma, vbrouwer and cschuijt, a project in the 42 common core.

## Considerations

* This project was written for Linux, so it uses `poll`. It is likely not possible to run on macOS.
* Codam does not enforce C++98 like other 42 campuses, so we have targeted C++20. The project will not compile if you target C++98.

### Config files
Our config parser stays close to what configurations in nginx look like and does not require a different format. However, it comes with a set of limitations to keep things manageable:
* One directive per line please, and no multi-line directives.
* Blocks should be opened on the same line as their parent directive, and closed on a separate line.
* Trailing comments are okay.
* Directives on the root level that are not `http` are ignored.
* Directives under the `http` directive that exist in nginx but are not implemented in webserv will raise a warning, but will not interrupt the program.
* Unknown directives will cause the program to error out.
* We do not support variables in the config file.

The following directives and arguments (from the [ngx core module](https://nginx.org/en/docs/http/ngx_http_core_module.html) and [ngx http autoindex module](https://nginx.org/en/docs/http/ngx_http_autoindex_module.html)) are implemented:
| Directive   |Arguments|Context|Notes  |
|-------------|---------|-------|-------|
|__`http`__| - |Root||
|__`server`__| - |`http`||
|__`server_name`__|`name`, `...`|`server`|Supports wildcards. Can pass `.example.com` to create both `example.com` and `*.example.com` names.|
|__`listen`__|`address[:port]`, `port`, `...`, `[default_server]`|`server`|Supports wildcards.|
|__`location`__ | `[=]`, `uri`|`server`, `location`|Wildcards are implemented, regex is not.|
|__`root`__|`path`|`http`, `server`, `location`|
|__`error_page`__|`code`, `uri`|`http`, `server`, `location`||
|__`client_max_body_size`__|`size`|`http`, `server`, `location`||
|__`autoindex`__|`on` or `off`|`http`, `server`, `location`||
|__`limit_except`__|`METHOD`, `...`|`location`|Granular filtering like in the ngx_http_access module is not required by the subject. Every directive like this will act as if `deny all;` is set under it.|
|__`return`__|`code [text]`|`server`, `location`|The text is the response body. This way, it is possible to redirect on a location: `return 301 https://differentsite.com;`|
|__`index`__|`file`, `...`|`http`, `server`, `location`|Files to try if the request is for a directory.|
