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


Notas para o funcionamento adequado:
Antes de rodar o programa, certifique-se de rodar primeiramente o nosso complemento que transforma as “,,” em “, ,“ para que o programa faça a leitura correta de todos os campos

Veja também o BUFFER_SIZE, sua memória principal deve aguentar BUFFER_SIZE * sizeof(Produto)
Um BUFFER_SIZE maior resultará em um tempo menor de execução das funções de ordenar e remover

Caso queira testar com poucos registros, na função que ordena o CSV nós temos um while que pode ser modificado para ler somente um número X de arquivos, modifique se quiser testar o programa rapidamente.













Código:

Estrutura arqUser
serial (CHAVE)
event_time
event_type
user_id
user_session
elo
excluido

Estrutura arqProd
serial
product_id (CHAVE)
category_id
category_code
brand
price
endereco
excluido
elo
