#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <limits>
#include <tuple>
#include <numeric>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;
const float INF = numeric_limits<float>::infinity();

struct Servico {
    string tipo;
    int id;
    int origem;
    int destino;
    int demanda;
    float custo;
};

struct DadosArquivo {
    int capacidade;
    int deposito;
    int num_nos;
    vector<vector<int>> nos;
    vector<vector<int>> arestas;
    vector<vector<int>> arcos;
};

void criar_diretorio(const string& caminho) {
    #ifdef _WIN32
    _mkdir(caminho.c_str());
    #else
    mkdir(caminho.c_str(), 0777);
    #endif
}

vector<string> obter_arquivos_dat() {
    vector<string> arquivos;
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir("arquivos")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string nome = ent->d_name;
            if (nome.size() > 4 && 
                nome.substr(nome.size() - 4) == ".dat") {
                arquivos.push_back(nome);
            }
        }
        closedir(dir);
    }
    sort(arquivos.begin(), arquivos.end());
    return arquivos;
}

vector<int> extrair_numeros(const string& s) {
    vector<int> numeros;
    stringstream ss(s);
    int num;
    while (ss >> num) numeros.push_back(num);
    return numeros;
}

pair<vector<vector<float>>, vector<vector<int>>> floyd_warshall(int num_nos, const vector<tuple<int, int, float>>& arcos) {
    vector<vector<float>> dist(num_nos + 1, vector<float>(num_nos + 1, INF));
    vector<vector<int>> pred(num_nos + 1, vector<int>(num_nos + 1, -1));

    for (int i = 0; i <= num_nos; ++i) dist[i][i] = 0;

    for (const auto& arco : arcos) {
        int u = get<0>(arco);
        int v = get<1>(arco);
        float w = get<2>(arco);
        dist[u][v] = w;
        pred[u][v] = u;
    }

    for (int k = 0; k <= num_nos; ++k) {
        for (int i = 0; i <= num_nos; ++i) {
            for (int j = 0; j <= num_nos; ++j) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    pred[i][j] = pred[k][j];
                }
            }
        }
    }
    return make_pair(dist, pred);
}

DadosArquivo ler_arquivo(const string& nome_arquivo) {
    DadosArquivo dados;
    ifstream arquivo("arquivos/" + nome_arquivo);
    
    if (!arquivo.is_open()) {
        throw runtime_error("Erro ao abrir arquivo: arquivos/" + nome_arquivo);
    }

    string linha;
    int secao = 0;

    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;

        if (linha.find("Capacity:") != string::npos) {
            vector<int> valores_cabecalho;
            while (getline(arquivo, linha) && !linha.empty()) {
                vector<int> nums = extrair_numeros(linha);
                valores_cabecalho.insert(valores_cabecalho.end(), nums.begin(), nums.end());
            }
            if (valores_cabecalho.size() >= 9) {
                dados.capacidade = valores_cabecalho[2];
                dados.deposito = valores_cabecalho[3];
                dados.num_nos = valores_cabecalho[4];
            }
            secao = 1;
        }
        else if (linha.find("ReN.") != string::npos) {
            getline(arquivo, linha);
            while (getline(arquivo, linha) && !linha.empty()) {
                vector<int> nums = extrair_numeros(linha);
                if (nums.size() >= 2) {
                    dados.nos.push_back({nums[0], nums[1]});
                }
            }
            secao = 2;
        }
        else if (linha.find("ReE.") != string::npos) {
            getline(arquivo, linha);
            while (getline(arquivo, linha) && !linha.empty()) {
                vector<int> nums = extrair_numeros(linha);
                if (nums.size() >= 5) {
                    dados.arestas.push_back({nums[1], nums[2], nums[3], nums[4]});
                }
            }
            secao = 3;
        }
        else if (linha.find("ReA.") != string::npos) {
            getline(arquivo, linha);
            while (getline(arquivo, linha) && !linha.empty()) {
                vector<int> nums = extrair_numeros(linha);
                if (nums.size() >= 5) {
                    dados.arcos.push_back({nums[1], nums[2], nums[3], nums[4]});
                }
            }
        }
    }

    return dados;
}

vector<Servico> criar_servicos(
    const vector<vector<int>>& nos,
    const vector<vector<int>>& arestas,
    const vector<vector<int>>& arcos,
    const vector<vector<float>>& dist,
    int deposito) {
    
    vector<Servico> servicos;
    int id = 1;
    
    for (const auto& n : nos) 
        servicos.push_back({"no", id++, n[0], n[0], n[1], 0});
    
    for (const auto& e : arestas)
        servicos.push_back({"aresta", id++, e[0], e[1], e[3], static_cast<float>(e[2])});
    
    for (const auto& a : arcos)
        servicos.push_back({"arco", id++, a[0], a[1], a[3], static_cast<float>(a[2])});
    
    sort(servicos.begin(), servicos.end(), [&](const Servico& a, const Servico& b) {
        return (dist[deposito][a.origem] + a.custo)/a.demanda < (dist[deposito][b.origem] + b.custo)/b.demanda;
    });
    
    return servicos;
}

vector<vector<Servico>> construir_rotas(vector<Servico> servicos, int capacidade, int deposito, const vector<vector<float>>& dist) {
    vector<vector<Servico>> rotas;
    
    while (!servicos.empty()) {
        vector<Servico> rota;
        int carga_atual = 0;
        int no_atual = deposito;
        
        while (true) {
            auto it = min_element(servicos.begin(), servicos.end(),
                [&](const Servico& a, const Servico& b) {
                    return dist[no_atual][a.origem] < dist[no_atual][b.origem];
                });
            
            if (it == servicos.end() || (carga_atual + it->demanda) > capacidade) break;
            
            rota.push_back(*it);
            carga_atual += it->demanda;
            no_atual = it->destino;
            servicos.erase(it);
        }
        
        if (!rota.empty()) rotas.push_back(rota);
    }
    
    return rotas;
}

void salvar_solucao(const string& nome_arquivo, const vector<vector<Servico>>& rotas, 
                  const vector<vector<float>>& dist, int deposito) {
    criar_diretorio("G21");
    ofstream saida("G21/sol-" + nome_arquivo);
    
    if (!saida.is_open()) {
        throw runtime_error("Erro ao criar arquivo de saida: G21/sol-" + nome_arquivo);
    }
    
    float custo_total = 0;
    for (const auto& rota : rotas) {
        float custo = 0;
        int ultimo = deposito;
        for (const auto& s : rota) {
            custo += dist[ultimo][s.origem] + s.custo;
            ultimo = s.destino;
        }
        custo += dist[ultimo][deposito];
        custo_total += custo;
    }
    
    saida << custo_total << "\n" << rotas.size() << "\n0\n0\n";
    
    for (const auto& rota : rotas) {
        saida << "0 1 " << &rota - &rotas[0] << " ";
        int demanda = accumulate(rota.begin(), rota.end(), 0, 
            [](int soma, const Servico& s) { return soma + s.demanda; });
        
        saida << demanda << " ";
        float custo = 0;
        int ultimo = deposito;
        for (const auto& s : rota) {
            custo += dist[ultimo][s.origem] + s.custo;
            ultimo = s.destino;
        }
        custo += dist[ultimo][deposito];
        saida << custo << " " << rota.size() + 2 << " ";
        
        saida << "(D 0," << deposito << "," << deposito << ") ";
        for (const auto& s : rota) 
            saida << "(S " << s.id << "," << s.origem << "," << s.destino << ") ";
        saida << "(D 0," << deposito << "," << deposito << ")\n";
    }
}

int main() {
    try {
        criar_diretorio("arquivos");
        criar_diretorio("G21");
        
        vector<string> arquivos = obter_arquivos_dat();
        
        if (arquivos.empty()) {
            cout << "Nenhum arquivo .dat encontrado no diretorio 'arquivos'!\n";
            cout << "Crie o diretorio 'arquivos' e adicione os arquivos de entrada.\n";
            return 1;
        }
        
        cout << "Arquivos disponiveis:\n";
        for (size_t i = 0; i < arquivos.size(); ++i) {
            cout << " [" << i + 1 << "] " << arquivos[i] << '\n';
        }
        
        int escolha;
        cout << "\nDigite o numero do arquivo para processar: ";
        cin >> escolha;
        
        if (escolha < 1 || escolha > static_cast<int>(arquivos.size())) {
            cout << "Escolha invalida!\n";
            return 1;
        }
        
        string arquivo_selecionado = arquivos[escolha - 1];
        cout << "\nProcessando arquivo: " << arquivo_selecionado << endl;
        
        DadosArquivo dados = ler_arquivo(arquivo_selecionado);
        cout << "Arquivo processado com sucesso!\n";
        
        vector<tuple<int, int, float>> todos_arcos;
        for (const auto& e : dados.arestas) {
            todos_arcos.push_back(make_tuple(e[0], e[1], e[2]));
            todos_arcos.push_back(make_tuple(e[1], e[0], e[2]));
        }
        for (const auto& a : dados.arcos) {
            todos_arcos.push_back(make_tuple(a[0], a[1], a[2]));
        }

        auto resultado = floyd_warshall(dados.num_nos, todos_arcos);
        cout << "Matriz de distancias calculada!\n";
        
        vector<Servico> servicos = criar_servicos(dados.nos, dados.arestas, dados.arcos, resultado.first, dados.deposito);
        cout << "Servicos criados: " << servicos.size() << endl;
        
        vector<vector<Servico>> rotas = construir_rotas(servicos, dados.capacidade, dados.deposito, resultado.first);
        cout << "Rotas construidas: " << rotas.size() << endl;
        
        salvar_solucao(arquivo_selecionado, rotas, resultado.first, dados.deposito);
        cout << "\n=== Processamento concluido com sucesso! ===\n";
        cout << "Arquivo de saida gerado em: G21/sol-" << arquivo_selecionado << endl;
    }
    catch (const exception& e) {
        cerr << "\nErro: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
