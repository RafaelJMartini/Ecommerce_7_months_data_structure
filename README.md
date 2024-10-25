CONTEXTO
Primeiramente alteramos o arquivo original para que no lugar de ,, tenhamos , , pois a forma como tokenizamos (strtok) não estava pegando todos os elementos

Após isso é feito a leitura desse arquivo alterado e separamos o que é uma informação do usuário e o que é do produto. Fazemos isso enquanto colocamos uma variável serial, que será nossa chave primária de usuários e nos ajudará a indexar o arquivo de produtos
Com ambos arquivos devidamente separados, temos que excluir as repetições existentes no arquivo de produtos e indexar com o arquivo de usuários por meio do campo endereço
Isso é feito carregando uma parte dos registros na memória, ordenando por product id e depois intercalando
Com esse novo arquivo de produtos, agora ordenado comparamos as entradas, se for a primeira vez do produto, escrevemos ele em outro arquivo e colocamos o endereço do primeiro usuário que pesquisou
Caso contrário usamos o endereço do último usuário que pesquisou e colocamos um elo para o próximo usuário que pesquisou por aquele ID
Com isso conseguimos realizar  a pesquisa por um produto, pesquisa pelos usuários que interagiram com um produto e pesquisar um acesso utilizando o serial
Para adição nós colocamos no final do arquivo de usuário e ligamos o produto que ele pesquisou para o último usuário que pesquisou ser ele
E no arquivo de produtos usamos uma área de extensão que ao fim do programa reorganiza os arquivos
Para remoção utilizamos a remoção lógica, no aquivo de produtos não removemos nada, no arq de usuários removemos logicamente e depois reorganizamos


Funções de Menu e Auxiliares:
  menu(): Exibe um menu com as opções disponíveis para o usuário. Essas opções envolvem a extração de dados, visualização de arquivos, verificação de registros, consulta de produtos, etc.
  
  comparar(): Função de comparação usada pelo qsort para ordenar produtos pelo product_id. Retorna um valor baseado na ordem alfabética dos product_ids.

  
Funções para Manipulação de Arquivos e Ordenação:
  ordena(): Ordena uma partição de produtos lida em memória usando qsort e salva em um arquivo temporário. Essa função é chamada várias vezes para dividir um grande arquivo em partes menores, ordenando cada uma delas        antes de realizar um merge.
  
  merge_files(): Realiza a fusão (merge) dos arquivos temporários ordenados em um arquivo final, mantendo a ordenação. Usa um array de buffers para ler uma entrada de cada arquivo temporário e grava as entradas em ordem     no arquivo final.

  
Funções para Exclusão e Compactação:
  excluirUsuarioPorId(): Marca um usuário como excluído de forma lógica, mudando o campo excluido para 1 em vez de removê-lo fisicamente.
  
  compactarArquivo(): Remove fisicamente os registros marcados como excluídos de um arquivo binário de usuários. Cria um novo arquivo, apenas com os usuários ativos, substituindo o arquivo original.


Funções de Leitura e Escrita de Dados:    
  preencherBinario(): Grava os dados de um usuário e de um produto em seus respectivos arquivos binários.
  
  verifica_arq(): Abre um arquivo binário e conta quantos registros ele possui. Exibe o valor do contador, útil para verificar a quantidade de registros armazenados.
  
  lerArquivoBin(): Lê um arquivo binário (de usuários ou produtos) e exibe os dados no console, dependendo do parâmetro aux (se aux == 0, lê User; caso contrário, lê Produto).
  
  Extrai_csv(): Extrai dados de um arquivo CSV e grava esses dados em arquivos binários separados para usuários e produtos. Essa função faz a leitura linha por linha, usa strtok para separar os campos e os armazena nas      structs correspondentes.

  
Funções para Ordenação e Remoção de Duplicados:
  OrdenaEMerge(): Chama ordena() para criar partições e, em seguida, merge_files() para mesclar os arquivos ordenados, removendo os arquivos temporários ao final.
  
  hash(): Calcula um valor hash para uma string (product_id) usando uma função baseada no algoritmo djb2. Essa função é usada para determinar a posição de um product_id na tabela hash.
  
  jaExiste() e adicionarTabela(): Verificam e adicionam, respectivamente, um product_id à tabela hash, evitando duplicações ao processar produtos.
  
  remove_repetidos(): Remove produtos duplicados com base no product_id. Armazena os product_ids em uma tabela hash para rastrear duplicados e evita gravação de produtos repetidos. Se um product_id duplicado for       
  encontrado, ela atualiza o campo elo nos registros de usuário associados.

  
Função de Busca:
pesquisa_binaria(): Realiza uma pesquisa binária em um arquivo de produtos binário ordenado para localizar um product_id específico. Retorna o produto encontrado, facilitando a consulta eficiente.



Notas para o funcionamento adequado:
Antes de rodar o programa, certifique-se de rodar primeiramente o nosso complemento que transforma as “,,” em “, ," para que o programa faça a leitura correta de todos os campos

Veja também o BUFFER_SIZE, sua memória principal deve aguentar BUFFER_SIZE * sizeof(Produto)
Um BUFFER_SIZE maior resultará em um tempo menor de execução das funções de ordenar e remover

Caso queira testar com poucos registros, na função que ordena o CSV nós temos um while que pode ser modificado para ler somente um número X de arquivos, modifique se quiser testar o programa rapidamente.













Código:

Estrutura arqUser
serial (CHAVE) |
event_time |
event_type |
user_id |
user_session |
product_id |
elo | 
excluido | 

Estrutura arqProd
serial |
product_id (CHAVE) | 
category_id |
category_code |
brand | 
price |
endereco |
elo |
