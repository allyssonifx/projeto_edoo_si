// Aqui vão alguns includes da biblioteca FLTK que eu usei para fazer a interface gráfica
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <iostream>
#include <string>
#include <sstream>
// Includes de algumas classes usadas no projeto
#include "src/download.h"
#include "src/conexao_banco.cpp"
#include "src/zipper.cpp"
#include <algorithm>
#include <cctype>

//A função dessa classe é criar um objeto que contenha 3 campos de saída e um botão para interação
class Monitoramento {
public:
    //Outputs
    Fl_Output* registros;
    Fl_Output* area;
    Fl_Output* status;
    //variáveis utilizadas
    std::string link;
    std::string arquivo;
    std::string titulo;
    //Objeto da classe de download
    Downloader downloader;
    //Objeto da classe de leitura do arquivo .shp
    ShapefileReader reader;
    //Objeto da classe de conexão com banco de dados
    PostGISConnection& dbconnection;

    Monitoramento(int x, int y, const std::string& titulo, const std::string& link, const std::string& arquivo, const Downloader& downloader, const ShapefileReader& reader, PostGISConnection& dbconnection)
        : link(link), arquivo(arquivo), downloader(downloader), reader(reader), dbconnection(dbconnection), titulo(titulo)
    {   
        //É consultado o link de download do arquivo armazenado no banco de dados.
        this->link = this->dbconnection.retornar_item("link", titulo);
        //O mesmo ocorre para o nome do arquivo
        this->arquivo = "arquivos/" + this->dbconnection.retornar_item("nome_arquivo", titulo);
        //Criação do título do objeto
        Fl_Box* titulo_box = new Fl_Box(438, y, 400, 30, titulo.c_str());
        titulo_box->labelsize(18);
        titulo_box->labelfont(FL_BOLD);
        titulo_box->labelcolor(FL_WHITE);
        titulo_box->color(FL_DARK_GREEN); 
        //Criação dos outputs
        registros = new Fl_Output(x, y + 60, 130, 30, "Registros:");
        area = new Fl_Output(x + 300, y + 60, 130, 30, "Área ha total:");
        status = new Fl_Output(x + 600, y + 60, 130, 30, "Status:");
        //Aqui alguns dados são consultados para preencher o output inicialmente como, calculo da área do dado total e quantidade de registros
        std::string tabela = titulo;
        std::transform(tabela.begin(), tabela.end(), tabela.begin(), [](unsigned char c) { return std::tolower(c); });
        std::string valor_area = this->dbconnection.retornar_area(tabela);
        std::string qtd_registros = this->dbconnection.retornar_tamanho(tabela);
        //configuração dos outputs
        registros->value(qtd_registros.c_str());
        area->value(valor_area.c_str());
        status->value("OK");
        registros->labelcolor(FL_WHITE);
        area->labelcolor(FL_WHITE);
        status->labelcolor(FL_WHITE);
        //Criação do botão para atualizar os dados
        Fl_Button* btn_atualizar = new Fl_Button(x + 800, y + 60, 100, 30, "Atualizar");
        btn_atualizar->color(FL_DARK_GREEN); 
        btn_atualizar->labelcolor(FL_WHITE); 
        btn_atualizar->labelsize(14);
        btn_atualizar->box(FL_ROUNDED_BOX); 
        //Ao clicar no botão uma série de funções são ativadas visando atualizar o dado no banco de dados.
        btn_atualizar->callback([](Fl_Widget*, void* data) {
            //Armazena os dados da classe para serem usados dentro do callback
            Monitoramento* monitor = static_cast<Monitoramento*>(data);
            monitor->status->value("Aguardando...");
            //Faz o download do arquivo
            monitor->downloader.baixarArquivo(monitor->link, monitor->arquivo);
            //pega o caminho final do arquivo .shp
            std::string pasta = monitor->arquivo.substr(0, monitor->arquivo.size() - 4);
            std::string caminho_arquivo = monitor->dbconnection.retornar_item("caminho_arquivo", monitor->titulo);
            //descompacta o arquivo .zip baixado
            std::string tabela = monitor->titulo;
            std::transform(tabela.begin(), tabela.end(), tabela.begin(), [](unsigned char c) { return std::tolower(c); });
            unzip(monitor->arquivo, pasta);
            //Lê o arquivo shapefile
            monitor->reader.readShapefile(caminho_arquivo);
            //Transfere o arquivo para o banco de dados
            monitor->reader.processShapefile(monitor->dbconnection, tabela);
            //Refaz os cálculos dos outputs
            std::string valor_area = monitor->dbconnection.retornar_area(tabela);
            std::string qtd_registros = monitor->dbconnection.retornar_tamanho(tabela);
            monitor->registros->value(qtd_registros.c_str());
            monitor->area->value(valor_area.c_str());
            monitor->status->value("Atualizado");
        }, (void*)this); 
    }
};

//Essa classe é responsável por criar o objeto que recebe um poligono como entrada e retorna se ele tem problemas com desmatamento.
class AnaliseTerreno {
public:
    //Criação dos objetos do GUI e da conexão com o banco
    Fl_Input* input;
    Fl_Output* output;
    PostGISConnection& dbconnection;

    AnaliseTerreno(int x, int y, PostGISConnection& dbconnection)
        : dbconnection(dbconnection)
    {   
        // Cria e configura os objetos como input, output e botão
        input = new Fl_Input(x, y + 40, 400, 30, "Digite a análise:");
        output = new Fl_Output(x, y + 80, 400, 30, "Resultado:");

        input->labelcolor(FL_WHITE);
        output->labelcolor(FL_WHITE);

        Fl_Button* btn_enviar = new Fl_Button(x + 140, y + 120, 100, 30, "Enviar");
        btn_enviar->color(FL_DARK_GREEN);
        btn_enviar->labelcolor(FL_WHITE);
        btn_enviar->labelsize(14);
        btn_enviar->box(FL_ROUNDED_BOX);
        //Callback para executar as funções de cálculo de intersecção
        btn_enviar->callback([](Fl_Widget*, void* data) {
            AnaliseTerreno* analise = static_cast<AnaliseTerreno*>(data);
            //Faz o cálculo de intersecção
            std::string wkt = analise->input->value();
            std::string resposta = analise->dbconnection.retornar_intersecao(wkt);
            //Retorna o resultado
            analise->output->value(resposta.c_str());
        }, (void*)this);
    }
};

int main() {
    //Configuração da tela como geral.
    Fl_Window* window = new Fl_Window(1280, 720, "Monitoramento Desmatamento");

    window->color(FL_BLACK);
    window->labelcolor(FL_WHITE);
    //Informações sobre o banco de dados postgresql
    std::string conexao = "host=localhost port=5432 dbname=projeto_si user=postgres password=VEGA123";
    Downloader downloader;
    ShapefileReader reader;
    PostGISConnection dbConnection(conexao);
    //Título da div
    Fl_Box* divisoria_desmat = new Fl_Box(50, 10, 1180, 30, "Dados de Desmatamento");
    divisoria_desmat->labelsize(20);
    divisoria_desmat->labelfont(FL_BOLD);
    divisoria_desmat->labelcolor(FL_WHITE);
    divisoria_desmat->color(FL_DARK_GREEN); 
    //Criação dos objetos de monitoramento tanto para Cerrado quanto amazônia.
    Monitoramento cerrado(80, 50, "DETER_CERRADO", "x", "arquivos/deter_cerrado.zip", downloader, reader, dbConnection);

    Monitoramento amazonas(80, 180, "DETER_AMAZONIA","x","arquivos/deter_amazonia.zip",downloader,reader,dbConnection);
    //Mais duas divs
    Fl_Box* divisoria = new Fl_Box(50, 320, 1180, 10, "_____________________________________________________________________________________________________________");
    divisoria->box(FL_FLAT_BOX);
    divisoria->color(FL_GRAY);

    Fl_Box* divisoria_title = new Fl_Box(50, 330, 1180, 30, "Análise de terreno");
    divisoria_title->labelsize(20);
    divisoria_title->labelfont(FL_BOLD);
    divisoria_title->labelcolor(FL_WHITE);
    divisoria_title->color(FL_DARK_GREEN); 
    //Criação do Objeto da análise de terreno
    AnaliseTerreno analise(450, 370, dbConnection);

    window->end();
    window->show();

    return Fl::run();
}
