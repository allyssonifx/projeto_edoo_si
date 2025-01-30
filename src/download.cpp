#include "download.h"
#include <fstream>
#include <iostream>

// Callback do libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* stream = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    stream->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}

// Essa classe é responsável por baixar o arquivo direto do site
Downloader::Downloader() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

Downloader::~Downloader() {
    curl_global_cleanup();
}
//A função faz todo o trabalho principal, recebe como entrada o link e o caminho da pasta
void Downloader::baixarArquivo(const std::string& url, const std::string& outputPath) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Erro ao inicializar o CURL" << std::endl;
        return;
    }

    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para escrita: " << outputPath << std::endl;
        curl_easy_cleanup(curl);
        return;
    }

    //Aqui estão as funções da biblioteca que baixam e escrevem o arquivo
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Erro ao baixar o arquivo: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
    file.close();
}
