#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>
#include <iomanip>

using namespace std;

// Estruturas de dados
struct Aresta {
    int origem;
    int destino;
    int custo;
    bool operator<(const Aresta& outra) const {
        if (origem != outra.origem) return origem < outra.origem;
        if (destino != outra.destino) return destino < outra.destino;
        return custo < outra.custo;
    }
};

struct Arco {
    int origem;
    int destino;
    int custo;
    bool operator<(const Arco& outro) const {
        if (origem != outro.origem) return origem < outro.origem;
        if (destino != outro.destino) return destino < outro.destino;
        return custo < outro.custo;
    }
};

struct VerticeRequerido {
    int no;
    int demanda;
    int custo_servico;
    bool operator<(const VerticeRequerido& outro) const {
        if (no != outro.no) return no < outro.no;
        if (demanda != outro.demanda) return demanda < outro.demanda;
        return custo_servico < outro.custo_servico;
    }
};

struct ArestaRequerida {
    int origem;
    int destino;
    int custo;
    int demanda;
    int custo_servico;
    bool operator<(const ArestaRequerida& outra) const {
        if (origem != outra.origem) return origem < outra.origem;
        if (destino != outra.destino) return destino < outra.destino;
        if (custo != outra.custo) return custo < outra.custo;
        if (demanda != outra.demanda) return demanda < outra.demanda;
        return custo_servico < outra.custo_servico;
    }
};

struct ArcoRequerido {
    int origem;
    int destino;
    int custo;
    int demanda;
    int custo_servico;
    bool operator<(const ArcoRequerido& outro) const {
        if (origem != outro.origem) return origem < outro.origem;
        if (destino != outro.destino) return destino < outro.destino;
        if (custo != outro.custo) return custo < outro.custo;
        if (demanda != outro.demanda) return demanda < outro.demanda;
        return custo_servico < outro.custo_servico;
    }
};

struct DadosGrafo {
    unordered_set<int> vertices;
    set<Aresta> arestas;
    set<Arco> arcos;
    set<VerticeRequerido> vertices_requeridos;
    set<ArestaRequerida> arestas_requeridas;
    set<ArcoRequerido> arcos_requeridos;
};

// Funções auxiliares
vector<string> dividir_string(const string &s, char delimitador) {
    vector<string> partes;
    string parte;
    istringstream fluxo(s);
    while (getline(fluxo, parte, delimitador)) {
        partes.push_back(parte);
    }
    return partes;
}

string trim(const string &s) {
    size_t inicio = s.find_first_not_of(" \t\n\r");
    size_t fim = s.find_last_not_of(" \t\n\r");
    return (inicio == string::npos) ? "" : s.substr(inicio, fim - inicio + 1);
}

DadosGrafo ler_arquivo(const string& caminho_arquivo) {
    DadosGrafo dados;
    ifstream arquivo(caminho_arquivo);
    string linha;
    string secao;

    while (getline(arquivo, linha)) {
        linha = trim(linha);
        if (linha.empty()) continue;

        if (linha.rfind("ReN.", 0) == 0) { secao = "ReN"; continue; }
        if (linha.rfind("ReE.", 0) == 0) { secao = "ReE"; continue; }
        if (linha.rfind("EDGE", 0) == 0) { secao = "EDGE"; continue; }
        if (linha.rfind("ReA.", 0) == 0) { secao = "ReA"; continue; }
        if (linha.rfind("ARC", 0) == 0) { secao = "ARC"; continue; }

        vector<string> partes = dividir_string(linha, '\t');
        try {
            if (secao == "ReN" && partes.size() >= 3) {
                string no_str = partes[0];
                no_str.erase(remove(no_str.begin(), no_str.end(), 'N'), no_str.end());
                int no = stoi(no_str);
                dados.vertices_requeridos.insert({no, stoi(partes[1]), stoi(partes[2])});
                dados.vertices.insert(no);
            }
            else if ((secao == "ReE" || secao == "EDGE") && partes.size() >= 4) {
                int origem = stoi(partes[1]);
                int destino = stoi(partes[2]);
                dados.vertices.insert(origem);
                dados.vertices.insert(destino);
                Aresta e{min(origem, destino), max(origem, destino), stoi(partes[3])};
                dados.arestas.insert(e);
                
                if (secao == "ReE" && partes.size() >= 6) {
                    dados.arestas_requeridas.insert({e.origem, e.destino, e.custo, stoi(partes[4]), stoi(partes[5])});
                }
            }
            else if ((secao == "ReA" || secao == "ARC") && partes.size() >= 4) {
                int origem = stoi(partes[1]);
                int destino = stoi(partes[2]);
                dados.vertices.insert(origem);
                dados.vertices.insert(destino);
                Arco a{origem, destino, stoi(partes[3])};
                dados.arcos.insert(a);
                
                if (secao == "ReA" && partes.size() >= 6) {
                    dados.arcos_requeridos.insert({a.origem, a.destino, a.custo, stoi(partes[4]), stoi(partes[5])});
                }
            }
        } catch (...) {}
    }
    return dados;
}

struct Grau {
    int grau_arestas = 0;
    int grau_entrada = 0;
    int grau_saida = 0;
};

unordered_map<int, Grau> calcular_graus(const unordered_set<int>& vertices, const set<Aresta>& arestas, const set<Arco>& arcos) {
    unordered_map<int, Grau> graus;
    for (int v : vertices) graus[v];
    
    for (const Aresta& e : arestas) {
        graus[e.origem].grau_arestas++;
        graus[e.destino].grau_arestas++;
    }
    
    for (const Arco& a : arcos) {
        graus[a.destino].grau_entrada++;
        graus[a.origem].grau_saida++;
    }
    
    return graus;
}

int contar_componentes_conectados(const DadosGrafo& dados) {
    unordered_set<int> visitados;
    int componentes = 0;
    
    for (int vertice : dados.vertices) {
        if (visitados.find(vertice) == visitados.end()) {
            componentes++;
            queue<int> fila;
            fila.push(vertice);
            visitados.insert(vertice);
            
            while (!fila.empty()) {
                int atual = fila.front();
                fila.pop();
                
                for (const Aresta& e : dados.arestas) {
                    if (e.origem == atual && visitados.find(e.destino) == visitados.end()) {
                        visitados.insert(e.destino);
                        fila.push(e.destino);
                    }
                    if (e.destino == atual && visitados.find(e.origem) == visitados.end()) {
                        visitados.insert(e.origem);
                        fila.push(e.origem);
                    }
                }
                
                for (const Arco& a : dados.arcos) {
                    if (a.origem == atual && visitados.find(a.destino) == visitados.end()) {
                        visitados.insert(a.destino);
                        fila.push(a.destino);
                    }
                }
            }
        }
    }
    return componentes;
}

pair<unordered_map<int, double>, unordered_map<int, int>> calcular_caminhos_mais_curtos(int no_inicial, const set<Aresta>& arestas, const set<Arco>& arcos) {
    unordered_map<int, double> distancias;
    unordered_map<int, int> predecessores;
    set<pair<double, int>> fila_prioridade;
    
    distancias[no_inicial] = 0;
    fila_prioridade.insert({0, no_inicial});
    
    while (!fila_prioridade.empty()) {
        int atual = fila_prioridade.begin()->second;
        fila_prioridade.erase(fila_prioridade.begin());
        
        for (const Aresta& e : arestas) {
            int vizinho = -1;
            if (e.origem == atual) vizinho = e.destino;
            else if (e.destino == atual) vizinho = e.origem;
            if (vizinho == -1) continue;
            
            double nova_dist = distancias[atual] + e.custo;
            if (!distancias.count(vizinho) || nova_dist < distancias[vizinho]) {
                distancias[vizinho] = nova_dist;
                predecessores[vizinho] = atual;
                fila_prioridade.insert({distancias[vizinho], vizinho});
            }
        }
        
        for (const Arco& a : arcos) {
            if (a.origem == atual) {
                double nova_dist = distancias[atual] + a.custo;
                if (!distancias.count(a.destino) || nova_dist < distancias[a.destino]) {
                    distancias[a.destino] = nova_dist;
                    predecessores[a.destino] = atual;
                    fila_prioridade.insert({distancias[a.destino], a.destino});
                }
            }
        }
    }
    
    return {distancias, predecessores};
}

unordered_map<int, unordered_map<int, double>> criar_matriz_distancias(const unordered_set<int>& vertices, const set<Aresta>& arestas, const set<Arco>& arcos) {
    unordered_map<int, unordered_map<int, double>> matriz;
    for (int v : vertices) {
        auto resultado = calcular_caminhos_mais_curtos(v, arestas, arcos);
        matriz[v] = resultado.first;
    }
    return matriz;
}

unordered_map<int, unordered_map<int, int>> criar_matriz_predecessores(const unordered_set<int>& vertices, const set<Aresta>& arestas, const set<Arco>& arcos) {
    unordered_map<int, unordered_map<int, int>> matriz;
    for (int v : vertices) {
        auto resultado = calcular_caminhos_mais_curtos(v, arestas, arcos);
        matriz[v] = resultado.second;
    }
    return matriz;
}

vector<int> obter_caminho_mais_curto(const unordered_map<int, unordered_map<int, int>>& predecessores, int inicio, int fim) {
    vector<int> caminho;
    int atual = fim;
    while (atual != -1) {
        caminho.insert(caminho.begin(), atual);
        auto it = predecessores.at(inicio).find(atual);
        atual = (it != predecessores.at(inicio).end()) ? it->second : -1;
    }
    return caminho;
}

double calcular_diametro(const unordered_map<int, unordered_map<int, double>>& matriz) {
    double diametro = 0;
    for (const auto& linha : matriz) {
        for (const auto& coluna : linha.second) {
            if (coluna.second > diametro) diametro = coluna.second;
        }
    }
    return diametro;
}

double calcular_media_caminhos(int num_vertices, const unordered_map<int, unordered_map<int, double>>& matriz) {
    double total = 0;
    int contagem = 0;
    for (const auto& linha : matriz) {
        for (const auto& coluna : linha.second) {
            if (linha.first != coluna.first && coluna.second < numeric_limits<double>::infinity()) {
                total += coluna.second;
                contagem++;
            }
        }
    }
    return contagem > 0 ? total / contagem : 0;
}

unordered_map<int, int> calcular_intermediacao(const unordered_set<int>& vertices, const unordered_map<int, unordered_map<int, int>>& predecessores) {
    unordered_map<int, int> intermediacao;
    for (int u : vertices) {
        for (int v : vertices) {
            if (u != v) {
                auto caminho = obter_caminho_mais_curto(predecessores, u, v);
                for (size_t i = 1; i < caminho.size() - 1; i++) {
                    intermediacao[caminho[i]]++;
                }
            }
        }
    }
    return intermediacao;
}

void imprimir_metricas(const DadosGrafo& dados) {
    auto graus = calcular_graus(dados.vertices, dados.arestas, dados.arcos);
    
    int grau_total_min = numeric_limits<int>::max();
    int grau_total_max = 0;
    
    for (const auto& par : graus) {
        const Grau& g = par.second;
        int total = g.grau_arestas + g.grau_entrada + g.grau_saida;
        grau_total_min = min(grau_total_min, total);
        grau_total_max = max(grau_total_max, total);
    }
    
    double max_arestas = (dados.vertices.size() * (dados.vertices.size() - 1)) / 2.0;
    double max_arcos = dados.vertices.size() * (dados.vertices.size() - 1);
    double densidade = (dados.arestas.size() + dados.arcos.size()) / (max_arestas + max_arcos);
    
    auto matriz_distancias = criar_matriz_distancias(dados.vertices, dados.arestas, dados.arcos);
    auto matriz_predecessores = criar_matriz_predecessores(dados.vertices, dados.arestas, dados.arcos);
    
    double diametro = calcular_diametro(matriz_distancias);
    double media_caminhos = calcular_media_caminhos(dados.vertices.size(), matriz_distancias);
    
    auto intermediacao = calcular_intermediacao(dados.vertices, matriz_predecessores);
    int componentes = contar_componentes_conectados(dados);
    
    cout << fixed << setprecision(4);
    cout << "1 - Quantidade de vértices: " << dados.vertices.size() << endl;
    cout << "2 - Quantidade de arestas: " << dados.arestas.size() << endl;
    cout << "3 - Quantidade de arcos: " << dados.arcos.size() << endl;
    cout << "4 - Vértices requeridos: " << dados.vertices_requeridos.size() << endl;
    cout << "5 - Arestas requeridas: " << dados.arestas_requeridas.size() << endl;
    cout << "6 - Arcos requeridos: " << dados.arcos_requeridos.size() << endl;
    cout << "7 - Densidade do grafo: " << densidade << endl;
    cout << "8 - Componentes conectados: " << componentes << endl;
    cout << "9 - Grau mínimo: " << grau_total_min << endl;
    cout << "10 - Grau máximo: " << grau_total_max << endl;
    cout << "11 - Intermediação de cada vértice:" << endl;
    for (const auto& par : intermediacao) {
        cout << "  Vértice " << par.first << ": " << par.second << endl;
    }
    cout << fixed << setprecision(2);
    cout << "12 - Caminho médio: " << media_caminhos << endl;
    cout << "13 - Diâmetro: " << diametro << endl;
}

int main() {
    cout << "Digite o nome/caminho do arquivo .dat: ";
    string caminho_arquivo;
    getline(cin, caminho_arquivo);
    
    DadosGrafo dados = ler_arquivo(caminho_arquivo);
    imprimir_metricas(dados);
    
    return 0;
}
