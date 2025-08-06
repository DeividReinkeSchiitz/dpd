#include "problem.h"

// funcao que cria as "arestas" com os pesos (scores) entre as turmas e os profs que são da mesma area
void adaptive_edges(Professor *professores, Turma *turmas, int *covered, int n, int m){
    int nscore, pref, alfa, score;
    // percorrendo as turmas
    for(int t = 0; t < m; t++){
        // se a turma t ja foi coberta, va para a proxima
        if(covered[t] == 1) continue;
        
        // percorrendo os professores
        for(int p = 0; p < n; p++){
            nscore = professores[p].n;  // pegando a quant de turmas que sao da area do prof p
            // se o prof p ja atingiu a ch min anual, va para o prox
            if(professores[p].current_CH1 + professores[p].current_CH2 >= professores[p].CHmin) continue;
            
            // verificando se o prof p possui ch disponivel no semestre da turma t
            if(turmas[t].semestre == 1 && professores[p].current_CH1 + turmas[t].CH > professores[p].CHmax1) continue;
            if(turmas[t].semestre == 2 && professores[p].current_CH2 + turmas[t].CH > professores[p].CHmax2) continue;
             
            // nesse ponto, eu posso verificar se o prof p eh da area da turma t
            pref = check_preference(professores[p].preferencias, turmas[t].codigo, m);
            if(pref != 0){
                alfa = random_number(1, 5);
                score = calculaScore(pref, alfa);
                professores[p].Score[nscore].score = score;
                professores[p].Score[nscore].codigo_turma = t+1;
                professores[p].n += 1; // mais uma turma que o prof p pode ministrar
                //turmas[t].professores[turmas[t].n];
                turmas[t].n += 1; // mais um prof que pode ministrar a turma t
            }else if(check_area(professores[p].myareas, turmas[t].disciplina.myareas) == 1){
                alfa = random_number(1, 8);
                professores[p].Score[nscore].score = alfa;
                professores[p].Score[nscore].codigo_turma = t+1;
                professores[p].n += 1; // mais uma turma que o prof p pode ministrar
                //turmas[t].professores[turmas[t].n];
                turmas[t].n += 1; // mais um prof que pode ministrar a turma t            
            }
            
        }
        
    }
}

// =================


// funcao que cria as "arestas" com os pesos (scores) entre as turmas e os profs que são da mesma area  (FUNCAO ANTIGA)
void adaptive_edges(Professor *professores, Turma *turmas, int *covered, int n, int m){
   int nscore, pref, alfa, score;
   // percorre os professores
   for(int p = 0; p < n; p++){

      // se o prof i ja atingiu a carga horaria, va para o proximo
      if(check_ch(professores[p].current_CH1, professores[p].current_CH2, professores[p].CHmin) == 1) continue;
      nscore = 0;

      // percorre as turmas
      for(int t = 0; t < m; t++){
         // se a turma j ja foi coberta, va para a proxima
        if(covered[t] == 1) continue;

         // se o prof i nao possui ch suficiente no semestre da turma j, va para a proxima turma
         if(turmas[t].semestre == 1 && professores[p].current_CH1 + turmas[t].CH > professores[p].CHmax1) continue;
         if(turmas[t].semestre == 2 && professores[p].current_CH2 + turmas[t].CH > professores[p].CHmax2) continue;

         // verificado se a turma j foi escolhida pelo prof i
         pref = check_preference(professores[p].preferencias, turmas[t].codigo, m);
         if(pref != 0){
           //printf("PROF %s pode ministrar a turma %d\n", professores[i].nome, turmas[j].codigo);
            alfa = random_number(1, 5);
            score = calculaScore(pref, alfa);
            professores[p].Score[nscore].score = score;
            professores[p].Score[nscore].codigo_turma = t+1;
            nscore++;
            turmas[t].n++;

            // nao foi escolhida. verificando se a turma j é da area do prof i
         }else if(check_area(professores[p].myareas, turmas[t].disciplina.myareas) == 1){
            //printf("PROF %s pode ministrar a turma %d\n", professores[i].nome, turmas[j].codigo);
            alfa = random_number(1, 8);
            //score = calculaScore(0, alfa);  // score quando n tem pref eh so o alfa. posso tentar mudar isso depois
            professores[p].Score[nscore].score = alfa;
            professores[p].Score[nscore].codigo_turma = t+1;
            nscore++; 
            turmas[t].n++;
        }
      }

      professores[p].n = nscore;
   }

}
