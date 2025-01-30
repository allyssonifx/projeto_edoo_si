#include <iostream>
//Biblioteca leitura de shapefiles
#include <shapefil.h>
//Biblioteca de conexão com Banco PostgreSQL
#include <libpq-fe.h>
#include <sstream>
#include <string>

//Essa classe é responsável por todas as operações no banco de dados.
class PostGISConnection {
public:
    //Para funcionar ela precisa receber a string com as informações para conexão escrita no arquivo main.cpp
    explicit PostGISConnection(const std::string& connectionString) {
        conn = PQconnectdb(connectionString.c_str());
        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "Erro ao conectar ao banco: " << PQerrorMessage(conn) << std::endl;
            exit(1);
        }
    }

    // Impede cópia do objeto para evitar múltiplas conexões inválidas
    PostGISConnection(const PostGISConnection&) = delete;
    PostGISConnection& operator=(const PostGISConnection&) = delete;
    
    ~PostGISConnection() {
        PQfinish(conn);
    }
    //Essa função recebe a geometria um atributo e o nome da tabela para inserir uma linha no banco de dados
    public:
    void insertData(const std::string& wkt, const std::string& attribute, const std::string& tableName) 
    {
        // SQL de insert
        std::string sql = "INSERT INTO " + tableName + " (geom, atributo) VALUES (ST_GeomFromText('" +
                          wkt + "', 4674), '" + attribute + "');";
        //Executa o SQL
        PGresult* res = PQexec(conn, sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Erro ao inserir: " << PQerrorMessage(conn) << std::endl;
        }
        PQclear(res);
    }
    //Essa função retorna qualquer informação da tabela de metadados, basta passar o item/coluna e o codigo do dado
    public:
    std::string retornar_item(std::string item, std::string codigo) 
    {   
        std::string sql = "SELECT " + item + " FROM metadados WHERE codigo ='" + codigo + "'";
        std::cout << sql << std::endl;
        PGresult* res = PQexec(conn, sql.c_str());

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "Erro ao executar consulta: " << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            return "";
        }

        std::string valor = "";
        // Aqui apenas verifica se a resposta veio vazia ou não.
        if (PQntuples(res) > 0) {
            valor = PQgetvalue(res, 0, 0);
            std::cout << "valor obtido: " << valor << std::endl;
        }

        PQclear(res);
        return valor;
    }
    //Essa função faz a operação TRUNCATE da tabela, para atualização total
    public:
    void limpar_tabela(std::string tabela){
        std::string sql = "TRUNCATE TABLE "+tabela+";";
        std::cout << sql << std::endl;
        PGresult* res = PQexec(conn, sql.c_str());
        PQclear(res);
    }
    //Essa função retorna o tamanho de uma tabela
    std::string retornar_tamanho(std::string tabela){
        std::string qtd_registros = "0";
        std::string sql = "SELECT COUNT(*) FROM "+tabela+";";
        PGresult* res = PQexec(conn, sql.c_str());
        qtd_registros = PQgetvalue(res, 0, 0);
        PQclear(res);
        return qtd_registros;
    }

    //Essa função calcula a área de todas as geometrias e depois soma elas retornando a área total de desmatamento
    std::string retornar_area(std::string tabela) {
        std::string sql = "SELECT ROUND((SUM(ST_Area(ST_SetSRID(geom::geography,4674),true))/10000)::NUMERIC,2) AS area_ha FROM "+tabela+";";
        std::cout << sql << std::endl;
        PGresult* res = PQexec(conn, sql.c_str());
        std::string valor = "0.0 ha";
        std::cout << PQntuples(res) << std::endl;
        if (PQntuples(res) > 0) {
            // Obter o valor da primeira linha e primeira coluna
            valor = PQgetvalue(res, 0, 0);
            std::cout << "valor obtido: " << valor << std::endl;
            
        }
        PQclear(res);
        return valor;
    }
    //Essa função pega uma geometria e verifica se existe intersecção entre ela e uma das duas tabelas de desmatamento, se sim ela retorna a área dessa intersecção
    std::string retornar_intersecao(std::string wkt){
        std::string sql = "SELECT COALESCE(ROUND((SUM(ST_Area(ST_Intersection(combinado.geom, ST_GeomFromText('"+wkt+"', 4674)),true) / 10000))::NUMERIC,2),0) AS area_interseccao_ha FROM (SELECT geom FROM deter_cerrado UNION ALL SELECT geom FROM deter_amazonia) AS combinado WHERE ST_Intersects(combinado.geom, ST_GeomFromText('"+wkt+"', 4674))";
        std::cout << sql << std::endl;
        PGresult* res = PQexec(conn, sql.c_str());
        std::string valor = "Valores inválidos";
        std::cout << PQntuples(res) << std::endl;
        if (PQntuples(res) > 0) {
            valor = PQgetvalue(res, 0, 0);
            //Se a área for >0 ela retorna uma mensagem de alerta
            if (valor != "0") 
            {
            std::cout << "valor intersect: " << valor << std::endl;
            valor =  "ALERTA!!! Terreno com área desmatada de " +valor+" ha";
            }// Se for 0 ela avisa que o terreno está livre
            else{
                valor = "Terreno livre de problemas!";
            }
            
        }
        PQclear(res);
        return valor;
    }

private:
    PGconn* conn;
};

//Essa classe é responsável por fazer a manipulação do arquivo SHP.
class ShapefileReader {
public:
    ShapefileReader() : hSHP(nullptr), hDBF(nullptr), nEntities(0) {}
    ~ShapefileReader() {
        if (hSHP) {
            SHPClose(hSHP);
        }
        if (hDBF) {
            DBFClose(hDBF);
        }
    }

    // Função para ler o shapefile e recebe o caminho do shape como parâmetro
    void readShapefile(const std::string& shapefilePath) {
        //lê o arquivo .shp
        hSHP = SHPOpen(shapefilePath.c_str(), "rb");
        //lê o arquivo .dbf
        hDBF = DBFOpen(shapefilePath.c_str(), "rb");

        if (!hSHP || !hDBF) {
            std::cerr << "Erro ao abrir o shapefile." << std::endl; 
        }
        else {
            std::cout << "Leu bonitinho o shapefile." << std::endl;
        }

        SHPGetInfo(hSHP, &nEntities, nullptr, nullptr, nullptr);
    }

    // Função para processar o shapefile e inserir dados no banco de dados
    void processShapefile(PostGISConnection& dbConnection, const std::string& tableName) {
        //Faz a operação de TRUNCATE
        dbConnection.limpar_tabela(tableName);
        //Executa um for em todas as linhas do shapefile
        for (int i = 0; i < nEntities; ++i) {
            // pega a informação de geometria
            SHPObject* shape = SHPReadObject(hSHP, i);

            // Converter para o formato WKT
            std::string wkt = convertToWKT(shape);

            // Lê os dados do arquivo .dbf e seleciona uma coluna
            std::string attrValue = DBFReadStringAttribute(hDBF, i, 0);

            // Executa a função do insert no banco
            dbConnection.insertData(wkt, attrValue, tableName);

            SHPDestroyObject(shape);
        }
    }

private:
    SHPHandle hSHP;
    DBFHandle hDBF;
    int nEntities;

    // Função auxiliar para converter geometria em WKT
    std::string convertToWKT(SHPObject* shape) {
        std::ostringstream wkt;

        // Verificar se é Polygon ou MultiPolygon
        if (shape->nParts <= 1) {
            // Algorítimo para converter para Poligono wkt
            wkt << "POLYGON((";
            for (int i = 0; i < shape->nVertices; ++i) {
                wkt << shape->padfX[i] << " " << shape->padfY[i];
                if (i < shape->nVertices - 1) {
                    wkt << ", ";
                }
            }
            wkt << "))";
        } else {
            // Algorítimo para converter para MultiPolygon wkt
            wkt << "MULTIPOLYGON(";
            for (int part = 0; part < shape->nParts; ++part) {
                int startIdx = shape->panPartStart[part];
                int endIdx = (part == shape->nParts - 1) ? shape->nVertices : shape->panPartStart[part + 1];

                wkt << "((";
                for (int i = startIdx; i < endIdx; ++i) {
                    wkt << shape->padfX[i] << " " << shape->padfY[i];
                    if (i < endIdx - 1) {
                        wkt << ", ";
                    }
                }
                wkt << "))";
                if (part < shape->nParts - 1) {
                    wkt << ", ";
                }
            }
            wkt << ")";
        }

        return wkt.str();
    }
};



