import random
from dataclasses import dataclass
import argparse

materias = [
    "Cálculo", # 1
    "Física", # 2
    "Programação", # 3
    "Algoritmos", # 4
    "Estruturas de Dados", # 5
    "Banco de Dados", # 6
    "Redes de Computadores", # 7
    "Sistemas Operacionais", # 8
    "Engenharia de Software", # 9
    "Inteligência Artificial", # 10
    "Matemática Discreta", # 11
    "Probabilidade e Estatística", # 12
    "Teoria da Computação", # 13
    "Compiladores", # 14
    "Arquitetura de Computadores", # 15
    "Segurança da Informação", # 16
    "Computação Gráfica", # 17
    "Interação Humano-Computador", # 18
    "Desenvolvimento Web", # 19
    "Engenharia de Dados", # 20
    "Processamento de Imagens", # 21
    "Visão Computacional", # 22
    "Aprendizado de Máquina", # 23
    "Processamento de Linguagem Natural", # 24
    "Computação em Nuvem", # 25
    "Computação Paralela e Distribuída", # 26
    "Robótica", # 27
    "Sistemas Embarcados", # 28
    "Bioinformática", # 29
    "Computação Quântica", # 30
    "Realidade Virtual e Aumentada", # 31
    "Metaverso" # 32        
]

def main(NUMBER_OF_COURSES, RELATIONSHIP_COURSE_PROFESSOR):
    def f(x):
        from math import acos, pi
        return (acos(1 - 2*x)) / (pi)

    @dataclass
    class Materia:
        codigo: int
        semestre: int
        numero: str
        nome: str
        curso: str
        CH: int
        area: int

    materias_geradas = []
    quantidades_materias = []
    linhas_materias = []
    linhas_professores = []

    codigo = 1
    area = 1
    for materia in materias[:NUMBER_OF_COURSES]:
        n = random.randint(1, 10)
        quantidades_materias.append(n)
        for idx in range(1, n+1):
            semestre = 1
            numero = "T1"
            nome = f"{materia} {idx}"
            curso = f"{materia} {idx}"
            CH = random.randint(0, 100)
            m = Materia(codigo, semestre, numero, nome, curso, CH, area)
            materias_geradas.append(m)
            linha = f"{m.codigo};{m.semestre};{m.numero};{m.nome};{m.curso};{m.CH};{m.area}\n"
            linhas_materias.append(linha)
            codigo += 1
        area *= 10

    prof_idx = 1
    for i, materia in enumerate(materias[:NUMBER_OF_COURSES]):
        max_prof = int(max(1, quantidades_materias[i] // RELATIONSHIP_COURSE_PROFESSOR))
        n_prof = random.randint(1, max_prof)
        ch_total = sum(m.CH for m in materias_geradas if m.nome.startswith(materia))
        ch_prof = ch_total // n_prof if n_prof > 0 else 0
        area_prof = materias_geradas[codigo - sum(quantidades_materias[i:]) - 1].area if materias_geradas else 0
        codigos_materia = [m.codigo for m in materias_geradas if m.nome.startswith(materia)]
        professores = []
        local_prof_idx = 1
        for j in range(1, n_prof+1):
            nome_prof = f"Professor {local_prof_idx} {materia}"
            chmin1 = int(f(random.uniform(0, 1)) * ch_prof) if ch_prof > 0 else 0
            chmax1 = int(f(random.uniform(0, 1)) * (ch_total - chmin1) + chmin1) if ch_total > chmin1 else chmin1
            
            preferencias = min(random.randint(1, quantidades_materias[i]), len(codigos_materia))
            codigos_disponiveis = codigos_materia.copy()
            preferencias_list = []
            for _ in range(preferencias):
                if codigos_disponiveis:
                    codigo_materia = random.choice(codigos_disponiveis)
                    codigos_disponiveis.remove(codigo_materia)
                else:
                    codigo_materia = 0
                valor = random.randint(0, 10)
                preferencias_list.append((codigo_materia, valor))
            professores.append({
                'nome_prof': nome_prof,
                'chmin1': chmin1,
                'chmax1': chmax1,
                'preferencias': preferencias,
                'preferencias_list': preferencias_list,
                'area_prof': area_prof
            })
            local_prof_idx += 1
        soma_chmax1 = sum(p['chmax1'] for p in professores)
        if soma_chmax1 <= ch_total and n_prof > 0:
            ajuste = ch_total - soma_chmax1 + 1
            professores[-1]['chmax1'] += ajuste
        for p in professores:
            linhas_professores.append(f"{p['nome_prof']};{p['chmin1']};{p['chmax1']};0;{p['preferencias']};{p['area_prof']};\n")
            for codigo_materia, valor in p['preferencias_list']:
                linhas_professores.append(f"{codigo_materia};{valor};\n")

    # Cálculo dos totais
    qtd_materias = len(materias_geradas)
    qtd_professores = sum(1 for l in linhas_professores if l.startswith("Professor"))
    qtd_areas = len(set(m.area for m in materias_geradas))

    with open('saida.csv', 'w', encoding='utf-8') as f:
        f.write(f"{qtd_materias};{qtd_professores};{qtd_areas};\n")
        f.write('codigo;semestre;numero;nome;curso;CH;area\n')
        for linha in linhas_materias:
            f.write(linha)
        # JUMP LINE
        f.write('\n')
        f.write('nome;CHmin1;CHmax1;CHmax2;preferencias;areas\n')
        for linha in linhas_professores:
            f.write(linha)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate data for DPD problem.")
    parser.add_argument("number_of_courses", type=int, help=f"Number of courses to generate (1-{len(materias)}).")
    parser.add_argument("relationship_course_professor", type=float, help="Relationship course-professor (1.0-2.0).")
    args = parser.parse_args()

    if not (1 <= args.number_of_courses <= len(materias)):
        parser.error(f"NUMBER_OF_COURSES must be between 1 and {len(materias)}.")

    if not (1.0 <= args.relationship_course_professor <= 2.0):
        parser.error("RELATIONSHIP_COURSE_PROFESSOR must be between 1.0 and 2.0.")

    main(args.number_of_courses, args.relationship_course_professor)