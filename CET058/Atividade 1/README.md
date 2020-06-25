
# Atividade 1: Analisador Léxico  
  
*Universidade Estadual de Santa Cruz*  
*Disciplina: (CET-058) Compiladores*  
*Aluno: Ramon Darwich de Menezes*  
  
## Objetivos:  
* Analisar um arquivo e separar os tokens nele contidos  
  
## Estrategias Usadas:  
  
Em ambos os métodos, optou-se por uma leitura em dois passos:  
* [FIRST - STEP] Leitura do arquivo e filtragem:  
	* Cada linha no arquivo produz uma lista de strings, sendo cada string somente um simbolo
* [SECOND - STEP] Analise das linhas produzidas pelo primeiro passo.  
  
1. Método via Automato:  
	* Produção e linkagem de 4 automatas, cada um especializado numa função (Automato para números, letras, símbolos matemáticos e lógicos).  
somente um simbolo, separado por espaço e em letra minuscula.  
	* [FIRST - STEP]  
	* Analise da linha filtrada [SECOND - STEP]:  
		* Cada simbolo da lista é testando contra todos os autômatos, podendo acontecer os seguintes casos:  
		1. Algum automato reconhece a string:  
			* *Nesse caso, mais uma filtragem é realizada para confirmar o token percebido, a depender do token, e o tipo preciso é definido.*  
		2. Nenhum automato reconhece a string:  
			* *Nesse caso, o token é marcado como DESCONHECIDO (-UNKNOWN-).*  
	* Impressão na tela de todos os token lidos, separados por linha do arquivo de entrada.  
2. Método via Trie - Dicionario:  
	* Preparação do Dicionario a partir de um arquivo de Dicionario de Lexemas (.lxd)  
	* [FIRST - STEP]  
	* Analise da linha filtrada [SECOND - STEP]:  
	* Cada token é testado contra um dos seguintes casos abaixo:  
		1. Se o primeiro caractere do token for um símbolo matemático, então o token todo é um símbolo matemático E não existe na Trie ainda, sendo adicionado em seguida.  
		2. Se o primeiro caractere do token for um número, então o token todo é um número E não existe na Trie ainda, sendo adicionado em seguida.  
		3. Se não for nenhum dos casos acima, o token é considerado ser uma variável e adicionado na Trie, cada 'variável' nova como sendo um token unico.  
	* Toda inserção na Trie passa por um teste de validação com a string provida na chamada, chamada de 'chave', e se já houver uma entrada com a mesma chave, ela é somente ignorada.  
	* Impressão na tela de todos os token lidos, separados por linha do arquivo de entrada.
