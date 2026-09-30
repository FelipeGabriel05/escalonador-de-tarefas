#!/usr/bin/env python3
# Servidor local simples: liga a interface (HTML) ao seu programa em C.
# A interface manda os dados -> aqui a gente roda o C -> devolve o resultado.
#
# Como usar:
#   python servidor.py
# Depois abra no navegador: http://localhost:8000

import http.server
import json
import os
import subprocess
import sys

PASTA = os.path.dirname(os.path.abspath(__file__))
os.chdir(PASTA)

# Onde esta o codigo C (neste projeto, na pasta src/)
SRC = "src"

# Nome do executavel (Windows usa .exe)
EXE = "programa.exe" if os.name == "nt" else "./programa"


def compilar():
    """Compila o C uma vez, se ainda nao existir o executavel."""
    alvo = "programa.exe" if os.name == "nt" else "programa"
    if os.path.exists(alvo):
        return True
    print("Compilando o C...")
    fontes = [os.path.join(SRC, "main.c")]
    for pasta in (os.path.join(SRC, "escalonadores"), os.path.join(SRC, "leitura")):
        for f in os.listdir(pasta):
            if f.endswith(".c"):
                fontes.append(os.path.join(pasta, f))
    cmd = ["gcc", "-I", SRC, "-std=c11", *fontes, "-o", alvo]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print("ERRO ao compilar:\n", r.stderr)
        return False
    print("Compilado com sucesso.")
    return True


def rodar_c(opcao, processos, quantum, aging):
    """Escreve os dados, roda o C e devolve a saida de texto."""
    with open("entrada.txt", "w") as f:
        f.write(processos)
    with open("config.txt", "w") as f:
        f.write("quantum:%d\naging:%d\n" % (quantum, aging))

    r = subprocess.run([EXE, str(opcao), "entrada.txt"],
                       capture_output=True, text=True)
    return r.stdout


class Handler(http.server.SimpleHTTPRequestHandler):
    # nao deixa o navegador guardar a pagina em cache
    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_POST(self):
        if self.path != "/run":
            self.send_error(404)
            return
        tam = int(self.headers.get("Content-Length", 0))
        dados = json.loads(self.rfile.read(tam))

        saida = rodar_c(
            int(dados.get("opcao", 1)),
            dados.get("processos", ""),
            int(dados.get("quantum", 2)),
            int(dados.get("aging", 1)),
        )

        corpo = json.dumps({"saida": saida}).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(corpo)

    def log_message(self, *a):
        pass


if __name__ == "__main__":
    if not compilar():
        sys.exit(1)
    porta = 8000
    print("Servidor rodando em http://localhost:%d" % porta)
    print("(Ctrl+C para parar)")
    http.server.HTTPServer(("localhost", porta), Handler).serve_forever()