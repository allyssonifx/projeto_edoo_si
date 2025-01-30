# Projeto da Disciplina de EDOO

## Desenvolvido por

**Allysson Fellype Gomes Muniz (afgm2)**

## Objetivo

Este projeto tem como objetivo criar um sistema de monitoramento e análise de desmatamento.

## Funcionamento

O programa está conectado a um banco de dados que contém bases de desmatamento da Amazônia e do Cerrado, distribuídas em duas tabelas. Durante a execução, há duas opções disponíveis:

1. **Atualizar uma das bases:**

   - O programa acessa a internet para obter os dados mais recentes;
   - Baixa o arquivo shapefile correspondente;
   - Lê e atualiza o banco de dados com novas informações.

2. **Fazer a análise de um terreno:**

   - O usuário insere uma geometria em formato WKT (preferencialmente no sistema SIRGAS 2000);
   - O programa cruza as informações com os dados do banco;
   - Se houver interseção entre a geometria inserida e as áreas de desmatamento, a área correspondente será retornada.

## Interface

A interface do programa é simples e intuitiva, contendo poucas informações visuais. No entanto, pode haver dificuldades para compilar o projeto devido à dependência de diversas bibliotecas e do banco de dados PostgreSQL.

## Bibliotecas Externas Utilizadas

- **fltk** (GUI)
- **curl** (download)
- **shapefil** (leitura de arquivos .shp)
- **libpq** (conexão com o banco de dados)
- **minizip** (descompactação de arquivos ZIP)

## Compilação

Para compilar o projeto, foi utilizado **mingw64**, e o seguinte comando no compilador:

```sh
g++ -o app main.cpp src/download.cpp src/conexao_banco.cpp -lfltk -lcurl -lshp -lpq -lminizip -lz
```

## Observações

O desenvolvimento do projeto passou por diversas mudanças e desafios, principalmente na busca por bibliotecas adequadas e na integração de múltiplas classes em C++. Embora o código possa não seguir todas as boas práticas, houve um esforço para mantê-lo funcional e compreensível.

## Prints

A pasta **"prints"** contém imagens que ilustram o funcionamento do projeto. Abaixo está a descrição de cada uma:

- **print\_projeto\_1**: Interface básica exibindo a quantidade de registros e a área total de desmatamento (ha) por bioma. Inclui status da camada e botão para atualização. Também contém a área de análise do terreno com campos para input, output e envio.
- **print\_projeto\_2**: Exemplo de atualização dos dados. O status da camada é alterado, e a atualização do desmatamento na Amazônia é visível.
- **print\_projeto\_3**: Análise de uma geometria de imóvel rural do SICAR. O cruzamento de dados identifica a área desmatada (vermelho) e a área do imóvel (tracejado) no **print\_projeto\_9**.
- **print\_projeto\_4**: Exemplo similar ao anterior, mas com outra geometria. Representação visual no **print\_projeto\_12** (laranja para desmatamento e tracejado para imóvel).
- **print\_projeto\_5**: Exemplo de geometria sem interseção com áreas de desmatamento.
- **print\_projeto\_6**: Visualização de todas as tabelas utilizadas no banco de dados.
- **print\_projeto\_7**: Exemplo da tabela de metadados, onde são armazenadas informações como link e nome do arquivo.
- **print\_projeto\_8**: Exemplo da tabela de desmatamento, com coluna de geometria em formato binário.
- **print\_projeto\_10**: Exibição completa dos dados do Cerrado.
- **print\_projeto\_11**: Exibição completa dos dados da Amazônia.

---

Este README serve como guia para entender o propósito, funcionamento e estrutura do projeto. Qualquer dúvida ou sugestão, sinta-se à vontade para contribuir! 🚀

# projeto_edoo_si
# projeto_edoo_si
# projeto_edoo_si
