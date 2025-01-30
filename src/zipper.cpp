#include <iostream>
#include <fstream>
//Biblioteca utilizada para descompactção
#include <minizip/unzip.h>
#include <windows.h>

//Função criada para descompactar arquivo zip
bool unzip(const std::string& zipFile, const std::string& destFolder) {
    //Pega o diretório raiz e junta com o informado
    char cwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd);
    std::string zipFilePath = std::string(cwd) + "/" + zipFile;
    std::string destFolderPath = std::string(cwd) + "/" + destFolder;
    //Leitura do arquivo zip
    unzFile file = unzOpen(zipFile.c_str());
    if (file == nullptr) {
        std::cerr << "Erro ao abrir o arquivo ZIP." << std::endl;
        return false;
    }
    //Verificação de integridade do arquivo
    unz_global_info globalInfo;
    if (unzGetGlobalInfo(file, &globalInfo) != UNZ_OK) {
        std::cerr << "Erro ao obter informações do arquivo ZIP." << std::endl;
        unzClose(file);
        return false;
    }
    //For para retirar todos os arquivos de dentro do zip
    char buffer[4096];
    for (unsigned int i = 0; i < globalInfo.number_entry; ++i) {
        unz_file_info fileInfo;
        char fileName[256];
        if (unzGetCurrentFileInfo(file, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) != UNZ_OK) {
            std::cerr << "Erro ao obter informações do arquivo dentro do ZIP." << std::endl;
            unzClose(file);
            return false;
        }

        // Abre o arquivo dentro do ZIP
        if (unzOpenCurrentFile(file) != UNZ_OK) {
            std::cerr << "Erro ao abrir o arquivo dentro do ZIP." << std::endl;
            unzClose(file);
            return false;
        }

        // Abre o arquivo de destino para escrita
        std::string outputFile = destFolder + "/" + fileName;
        std::ofstream out(outputFile, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "Erro ao criar o arquivo de destino: " << outputFile << std::endl;
            unzClose(file);
            return false;
        }

        // Descompacta o arquivo
        int bytesRead;
        while ((bytesRead = unzReadCurrentFile(file, buffer, sizeof(buffer))) > 0) {
            out.write(buffer, bytesRead);
        }

        out.close();
        unzCloseCurrentFile(file);

        // Passa para o próximo arquivo
        if (i + 1 < globalInfo.number_entry && unzGoToNextFile(file) != UNZ_OK) {
            std::cerr << "Erro ao passar para o próximo arquivo dentro do ZIP." << std::endl;
            unzClose(file);
            return false;
        }
    }

    unzClose(file);
    return true;
}
