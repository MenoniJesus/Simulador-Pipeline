## Simulador Pipeline
Este projeto é um simulador de pipeline desenvolvido para a disciplina de Arquitetura de Computadores. O simulador foi criado com base em instruções fornecidas pelo professor.

Tecnologias Utilizadas:
- Linguagem de Programação: C
- IDE: Visual Studio Code
- Ferramenta de Compilação: MSYS2
##
<h3>Como Compilar e Executar:</h3>
Para compilar o projeto, utilize o comando abaixo no terminal do MSYS2:

`gcc -o arq-sim arq-sim.c lib.c -Wall` 

O comando acima gera o executável arq-sim.exe. Para rodar o simulador com um arquivo binário de teste, utilize:

`./arq-sim.exe testes/perfect-squares.bin`

O arquivo `perfect-squares.bin` pode ser substituído por qualquer outro arquivo binário disponível na pasta Testes.