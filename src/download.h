#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <string>
//Biblioteca utilizada para fazer o download através do link direto
#include <curl/curl.h>

// Declara a função de callback
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

// Classe Downloader responsável oeki diwbkiad
class Downloader {
public:
    Downloader();
    ~Downloader();
    void baixarArquivo(const std::string& url, const std::string& outputPath);
};

#endif
