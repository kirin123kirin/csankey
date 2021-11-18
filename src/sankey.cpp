
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include "csankey.hpp"
#include "cxxopts/include/cxxopts.hpp"

#if _WIN32 || _WIN64
#include <Windows.h>
#else
#include <unistd.h>
#endif

std::string gettmpdir() {
#if _WIN32 || _WIN64
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

std::string exec(const std::string cmd) {
    char buffer[128];
    std::string result = "";
#if _WIN32 || _WIN64
    FILE* pipe = _popen(cmd.data(), "r");
#else
    FILE* pipe = popen(cmd.data(), "r");
#endif
    if(!pipe)
        throw std::runtime_error("popen() failed!");
    try {
        while(fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch(...) {
#if _WIN32 || _WIN64
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        throw;
    }
#if _WIN32 || _WIN64
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
#if _WIN32 || _WIN64
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

#endif
    return ret;
}

int main(int argc, char** argv) {
    std::string outpath, default_outfile = "tmp_sankey.html";

#if _WIN32 || _WIN64
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
#if _WIN32 || _WIN64
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
#if _WIN32 || _WIN64
            exec("start " + outpath);
#else
            exec("open " + outpath);
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
