## Trabalho Prático de Algoritimo em Grafos

- Instituição: UFLA
- Professor: Mayron Cesar de Oliveira
- Disciplina: Algoritimos em Grafos (GCC - 218)
- Alunos: Samuel Aguiar Guimarães e Luiz Felipe de Souza Marques

 ---

## Projeto de Análise de Grafos e Cálculo de Métricas

Este projeto tem como objetivo ler uma instância de grafo a partir de um arquivo .dat e calcular diversas métricas importantes, tais como:
- Número de vértices, arestas, arcos, vértices requeridos, arestas requeridas e arcos requeridos.
- Densidade do grafo.
- Grau mínimo e máximo (para arestas e arcos).
- Diâmetro do grafo e caminho médio.
- Centralidade de intermediação (betweenness centrality) para cada vértice.

O projeto foi implementado em C++ utilizando bibliotecas da STL (Standard Template Library).

---

## Estrutura do Projeto

- Final.cpp: Código-fonte principal do projeto.
- README.md: Este arquivo, contendo informações sobre o projeto.
- Arquivos de instância .dat: Arquivos de entrada que seguem o formato especificado (ver seção "Formato do Arquivo de Entrada").

---

## Compilação

Utilize um compilador C++ (como o g++) para compilar o código. Por exemplo:

```bash
g++ -std=c++11 -O2 q2.cpp -o grafo_project
