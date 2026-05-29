#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <chrono>
#include <iomanip>

struct Implicante {
    std::string bits;
    std::set<int> mintermos;
    bool usado = false;
    bool operator<(const Implicante& o) const {
        if (bits != o.bits) return bits < o.bits;
        return mintermos < o.mintermos;
    }
    bool operator==(const Implicante& o) const { return bits == o.bits; }
};

std::string toBinary(int val, int n) {
    std::string s(n, '0');
    for (int i = n - 1; i >= 0; i--) { s[i] = (val & 1) ? '1' : '0'; val >>= 1; }
    return s;
}

int contarUns(const std::string& s) {
    int c = 0; for (char ch : s) if (ch == '1') c++; return c;
}

bool combinar(const Implicante& a, const Implicante& b, Implicante& res) {
    int n = a.bits.size(), difs = 0, pos = -1;
    for (int i = 0; i < n; i++) {
        if (a.bits[i] != b.bits[i]) {
            if (a.bits[i] == '-' || b.bits[i] == '-') return false;
            difs++; pos = i;
        }
        if (difs > 1) return false;
    }
    if (difs != 1) return false;
    res.bits = a.bits; res.bits[pos] = '-';
    res.mintermos = a.mintermos;
    res.mintermos.insert(b.mintermos.begin(), b.mintermos.end());
    res.usado = false;
    return true;
}

std::string toExpressao(const std::string& bits, const std::vector<std::string>& vars) {
    std::string expr;
    for (int i = 0; i < (int)bits.size(); i++) {
        if (bits[i] == '-') continue;
        if (!expr.empty()) expr += ".";
        if (bits[i] == '0') expr += "~";
        expr += vars[i];
    }
    return expr.empty() ? "1" : expr;
}

// Remove comentário inline (tudo após '#')
std::string removerComentario(const std::string& linha) {
    auto pos = linha.find('#');
    if (pos != std::string::npos) return linha.substr(0, pos);
    return linha;
}

// Trim
std::string trim(const std::string& s) {
    int a = 0, b = (int)s.size() - 1;
    while (a <= b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) a++;
    while (b >= a && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r')) b--;
    return (a <= b) ? s.substr(a, b - a + 1) : "";
}

struct FuncaoPLA {
    int numEntradas = 0, numSaidas = 1;
    std::vector<std::string> nomesEntradas, nomesSaidas;
    std::vector<int> mintermos, dontCares;
};

FuncaoPLA lerPLA(std::istream& in) {
    FuncaoPLA f;
    std::string linha;
    while (std::getline(in, linha)) {
        linha = trim(removerComentario(linha));
        if (linha.empty()) continue;

        if (linha[0] == '.') {
            std::istringstream ss(linha);
            std::string cmd; ss >> cmd;
            if      (cmd == ".i")   ss >> f.numEntradas;
            else if (cmd == ".o")   ss >> f.numSaidas;
            else if (cmd == ".ilb") { std::string v; while (ss >> v) f.nomesEntradas.push_back(v); }
            else if (cmd == ".ob")  { std::string v; while (ss >> v) f.nomesSaidas.push_back(v); }
            else if (cmd == ".e" || cmd == ".end") break;
            continue;
        }

        std::istringstream ss(linha);
        std::string entrada, saida;
        if (!(ss >> entrada >> saida)) continue;
        if ((int)entrada.size() != f.numEntradas) continue;

        // Expande don't cares na entrada
        std::vector<std::string> padroes = {entrada};
        for (int i = 0; i < (int)entrada.size(); i++) {
            if (entrada[i] == '-') {
                std::vector<std::string> novos;
                for (auto& p : padroes) {
                    std::string p0 = p, p1 = p;
                    p0[i] = '0'; p1[i] = '1';
                    novos.push_back(p0); novos.push_back(p1);
                }
                padroes = novos;
            }
        }

        for (auto& p : padroes) {
            int val = 0; bool ok = true;
            for (char c : p) {
                if (c == '0' || c == '1') val = (val << 1) | (c - '0');
                else { ok = false; break; }
            }
            if (!ok) continue;
            char s0 = saida[0];
            if      (s0 == '1') f.mintermos.push_back(val);
            else if (s0 == '-') f.dontCares.push_back(val);
            // s0 == '0' → ignora (maxtermo)
        }
    }

    if (f.nomesEntradas.empty())
        for (int i = 0; i < f.numEntradas; i++) f.nomesEntradas.push_back(std::string(1, 'A'+i));
    if (f.nomesSaidas.empty())
        f.nomesSaidas.push_back("F");

    auto dedup = [](std::vector<int>& v) {
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
    };
    dedup(f.mintermos); dedup(f.dontCares);
    return f;
}

std::vector<Implicante> quine_mccluskey(
    const std::vector<int>& mintermos,
    const std::vector<int>& dontCares,
    int numBits, bool verbose = false)
{
    if (mintermos.empty()) return {};

    std::vector<Implicante> atual;
    for (int m : mintermos) { Implicante i; i.bits = toBinary(m,numBits); i.mintermos={m}; atual.push_back(i); }
    for (int d : dontCares) { Implicante i; i.bits = toBinary(d,numBits); i.mintermos={d}; atual.push_back(i); }

    std::set<Implicante> primos;
    int rodada = 1;

    while (!atual.empty()) {
        if (verbose) std::cout << "  [Rodada " << rodada++ << "] " << atual.size() << " termos\n";

        std::map<int, std::vector<Implicante*>> grupos;
        for (auto& imp : atual) grupos[contarUns(imp.bits)].push_back(&imp);

        std::vector<Implicante> proxima;
        std::set<std::string> jaAdicionados;

        auto it = grupos.begin();
        while (it != grupos.end()) {
            auto prox = std::next(it);
            if (prox == grupos.end()) break;
            for (auto* a : it->second)
                for (auto* b : prox->second) {
                    Implicante novo;
                    if (combinar(*a, *b, novo)) {
                        a->usado = b->usado = true;
                        if (!jaAdicionados.count(novo.bits)) {
                            jaAdicionados.insert(novo.bits);
                            proxima.push_back(novo);
                        }
                    }
                }
            it++;
        }
        for (auto& imp : atual) if (!imp.usado) primos.insert(imp);
        atual = proxima;
    }

    std::set<int> mintSet(mintermos.begin(), mintermos.end());
    std::vector<Implicante> primosVec;
    for (auto& ip : primos) {
        bool ok = false;
        for (int m : ip.mintermos) if (mintSet.count(m)) { ok = true; break; }
        if (ok) primosVec.push_back(ip);
    }

    if (verbose) {
        std::cout << "  Implicantes primos: " << primosVec.size() << "\n";
        for (auto& p : primosVec) {
            std::cout << "    " << p.bits << " → {";
            for (int m : p.mintermos) std::cout << m << " "; std::cout << "}\n";
        }
    }

    std::set<int> cobertos;
    std::vector<Implicante> resultado;

    // Seleciona essenciais
    bool mudou = true;
    while (mudou) {
        mudou = false;
        for (int m : mintermos) {
            if (cobertos.count(m)) continue;
            std::vector<int> cand;
            for (int i = 0; i < (int)primosVec.size(); i++)
                if (primosVec[i].mintermos.count(m)) cand.push_back(i);
            if (cand.size() == 1) {
                bool jaEsta = false;
                for (auto& r : resultado) if (r.bits == primosVec[cand[0]].bits) { jaEsta = true; break; }
                if (!jaEsta) {
                    resultado.push_back(primosVec[cand[0]]);
                    for (int mm : primosVec[cand[0]].mintermos) cobertos.insert(mm);
                    mudou = true;
                }
            }
        }
    }

    // Heurística para os restantes
    std::vector<int> restantes;
    for (int m : mintermos) if (!cobertos.count(m)) restantes.push_back(m);

    while (!restantes.empty()) {
        int melhorIdx = -1, melhorScore = -1;
        for (int i = 0; i < (int)primosVec.size(); i++) {
            bool jaEsta = false;
            for (auto& r : resultado) if (r.bits == primosVec[i].bits) { jaEsta = true; break; }
            if (jaEsta) continue;
            int score = 0;
            for (int m : restantes) if (primosVec[i].mintermos.count(m)) score++;
            if (score > melhorScore) { melhorScore = score; melhorIdx = i; }
        }
        if (melhorIdx == -1 || melhorScore == 0) break;
        resultado.push_back(primosVec[melhorIdx]);
        for (int m : primosVec[melhorIdx].mintermos) cobertos.insert(m);
        restantes.clear();
        for (int m : mintermos) if (!cobertos.count(m)) restantes.push_back(m);
    }

    return resultado;
}

void escreverPLA(std::ostream& out, const FuncaoPLA& f, const std::vector<Implicante>& res) {
    out << ".i " << f.numEntradas << "\n.o " << f.numSaidas << "\n";
    if (!f.nomesEntradas.empty()) { out << ".ilb"; for (auto& v : f.nomesEntradas) out << " " << v; out << "\n"; }
    if (!f.nomesSaidas.empty())   { out << ".ob";  for (auto& v : f.nomesSaidas)   out << " " << v; out << "\n"; }
    out << ".p " << res.size() << "\n";
    for (auto& imp : res) out << imp.bits << " 1\n";
    out << ".e\n";
}

void processar(const std::string& nome, const FuncaoPLA& f, bool verbose) {
    std::cout << "\n╔══ " << nome << " ══\n";
    std::cout << "  Entradas: " << f.numEntradas
              << " | Mintermos: " << f.mintermos.size()
              << " | Don't cares: " << f.dontCares.size() << "\n";

    if (verbose) std::cout << "\n";

    auto t0 = std::chrono::high_resolution_clock::now();
    auto res = quine_mccluskey(f.mintermos, f.dontCares, f.numEntradas, verbose);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double,std::milli>(t1-t0).count();

    std::cout << "  Implicantes minimizados: " << res.size() << "\n";
    std::cout << "  Tempo: " << std::fixed << std::setprecision(4) << ms << " ms\n";

    if (res.empty()) {
        std::cout << "  " << f.nomesSaidas[0] << " = 0\n";
    } else {
        std::cout << "  " << f.nomesSaidas[0] << " = ";
        bool primeiro = true;
        for (auto& imp : res) {
            if (!primeiro) std::cout << " + ";
            std::cout << toExpressao(imp.bits, f.nomesEntradas);
            primeiro = false;
        }
        std::cout << "\n\n  PLA minimizado:\n";
        escreverPLA(std::cout, f, res);
    }
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    std::vector<std::string> arquivos;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "-v" || a == "--verbose") verbose = true;
        else arquivos.push_back(a);
    }

    if (arquivos.empty()) {
        FuncaoPLA f = lerPLA(std::cin);
        processar("stdin", f, verbose);
    } else {
        for (auto& arq : arquivos) {
            std::ifstream in(arq);
            if (!in) { std::cerr << "Erro: não foi possível abrir '" << arq << "'\n"; continue; }
            FuncaoPLA f = lerPLA(in);
            processar(arq, f, verbose);
        }
    }
    return 0;
}
