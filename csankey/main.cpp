
#include "csankey.hpp"

#include "../extern/cxxopts/include/cxxopts.hpp"

#define IS_WIN (_WIN32 || _WIN64)

#if IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#include <X11/Xlib.h>
#include <limits.h>
#endif

std::string gettmpdir() {
#if IS_WIN
    std::string temp_dir;
    char charPath[MAX_PATH];
    if(GetTempPath(MAX_PATH, charPath)) {
        temp_dir = charPath;
        temp_dir.erase(temp_dir.size() - 1);  // remove last separater.
    }
    return temp_dir;
#else
    char* tempdir = getenv("TMPDIR");
    return std::string(((tempdir == nullptr) ? "/tmp" : tempdir));
#endif
}

// thanks for http://stackoverflow.com/questions/478898
std::wstring exec(const std::wstring cmd) {
    wchar_t buffer[128];
    std::wstring result = L"";
#if IS_WIN
    FILE* pipe = _wpopen(cmd.data(), L"r");
#else
    FILE* pipe = wpopen(cmd.data(), L"r");
#endif
    if(!pipe)
        throw std::runtime_error("popen() failed!");
    try {
        while(fgetws(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch(...) {
#if IS_WIN
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        throw;
    }
#if IS_WIN
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}

template <typename Container>
int from_data(Container data, const std::string& outpath, int header = -1) {
    SankeyData<wchar_t> snk(data);
    if(header == -1)
        snk.parse();
    else
        snk.parse((bool)header);

    return snk.to_html(outpath);
}

int from_clipboard(const std::string& outpath, int header = -1) {
    int ret = 1;
#if IS_WIN
    if(!OpenClipboard(nullptr))
        throw std::runtime_error("Failed Read Clipboard Data.");

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if(hData == nullptr)
        throw std::runtime_error("Failed Read Clipboard Data.");

    wchar_t* buf = static_cast<wchar_t*>(GlobalLock(hData));
    if(buf == nullptr)
        throw std::runtime_error("Failed Read Clipboard Data.");

    auto data = csv::CsvVec(buf, L'\t', L'"');
    ret = from_data(data, outpath, header);

    GlobalUnlock(hData);
    CloseClipboard();

#else

#if __APPLE__
    auto buf = exec("pbpaste");
#elif __linux__
    auto buf = exec("xsel --clipboard");
#endif
    if(buf.empty())
        throw std::runtime_error("Failed Read Clipboard Data.");
    auto data = csv::CsvVec(buf, L'\t', L'"');
    ret = from_data(data, outpath, header);

#endif
    return ret;
}

int main(int argc, char** argv) {
    std::string outpath, default_outfile = "tmp_sankey.html";

#if IS_WIN
    const char sep = '\\';
#else
    const char sep = '/';
#endif

    cxxopts::Options op("sankey", "Build sankey D3.js diagram from csv or tsv data.");

    try {
        op.add_options()("o,outpath", "output HTML File Path.\noutdirectory $TMPDIR/" + default_outfile,
                         cxxopts::value<std::string>());
        op.add_options()("n,no_open", "Is Open HTML auto by Default Browser? (default auto open)",
                         cxxopts::value<bool>()->default_value("false"));
        op.add_options()("H,header", "is header? (default no header)", cxxopts::value<bool>()->default_value("false"));
        op.add_options()("w,well_formed", "is input csv data is well-formed table? (default nonwell-formed)",
                         cxxopts::value<bool>()->default_value("false"));
        op.add_options()("s,sep", "separator(delimiter) of csv data. (default `,`)",
                         cxxopts::value<std::string>()->default_value(std::string(",")));
        op.add_options()("quote", "quote charactor of csv data. (default `\"`)",
                         cxxopts::value<std::string>()->default_value(std::string("\"")));
        op.add_options()("h,help", "Print usage");

        auto opts = op.parse(argc, argv);

        if(opts.count("help")) {
            std::cerr << op.help() << std::endl;
            exit(0);
        }

        if(opts.count("outpath"))
            outpath = opts["outpath"].as<std::string>();
        else
            outpath = gettmpdir() + sep + default_outfile;

        int ret;
        bool header = opts["header"].as<bool>();
        const wchar_t delimiter = (wchar_t)opts["sep"].as<std::string>()[0];
        const wchar_t quote = (wchar_t)opts["quote"].as<std::string>()[0];

        if(opts.unmatched().empty()) {
#if IS_WIN
            int nopipe = _isatty(_fileno(stdin));
#else
            int nopipe = isatty(fileno(stdin));
#endif
            if(nopipe) {
                if(header)
                    ret = from_clipboard(outpath, header);
                else
                    ret = from_clipboard(outpath);
            } else {
                auto data = csv::CsvstdinVec(delimiter, quote);
                ret = from_data(data, outpath, header);
            }
            if(ret) {
                std::cerr << "\nFailed Build Output html ->\n\t" << outpath << std::endl;
                exit(ret);
            }

        } else {
            int i = 1;
            for(auto&& inp : opts.unmatched()) {
                auto data = csv::CsvfileVec(inp, delimiter, quote);
                std::string file = outpath;
                std::size_t pos = outpath.find_last_of(".htm");
                if(pos != std::string::npos)
                    file.insert(pos, "_arg_" + std::to_string(i));

                ret = from_data(data, file, header);
                if(ret) {
                    std::cerr << "\nFailed Build Output html ->\n\t" << file << std::endl;
                    exit(ret);
                }
            }
        }

        bool auto_open = !opts["no_open"].as<bool>();

        if(auto_open) {
#if IS_WIN
            exec(L"start " + std::wstring(outpath.begin(), outpath.end()));
#else
            exec(L"open " + std::wstring(outpath.begin(), outpath.end()));
#endif
            std::cerr << "Opening Build Sankey Output html..." << std::endl;
            std::cerr << outpath << std::endl;
        }
        std::cerr << "Success!" << std::endl;
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        exit(1);
    }
    return 0;
}
