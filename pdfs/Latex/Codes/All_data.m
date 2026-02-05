% Código de exemplo - Apresentado na dissertação do Gabriel Gentil
clear all
%% Parametros de conversor CC-CC Boost
tam = 30;                           %quantidade de amostras
R = 48.3;                           %carga do conversor 
Vi = 10;                            %tensao de entrada
VoutRef = 24;                       %tensao de referencia 
T = 1e-6;                           %tempo discreto
tfinal = 0.32;                      %tempo final
tmedio = 0.04;                      %tempo do step no duty
tempo = tmedio:T:2*tmedio-T;        %vetor de tempo  

amostra = tfinal/T + 1;               %quantida de pontos
F = 40e3 + randn(tam,1)*100;          %Frequencia  
L = 210e-6 + randn(tam,1)*1e-6;       %Indutancia
C = 220e-6 + randn(tam,1)*1e-6;       %Capacitancia
D = .5 + randn(tam,1)*0.01;           %Duty
RL = 0.001 + randn(tam,1)*0.0001;     %Resistencia do indutor
RC = 0.001 + randn(tam,1)*0.0001;     %Resistencia do capacitor
VD = 0.8 + randn(tam,1)*0.01;         %Queda de tensao do diodo
RDIODE = 0.001 + randn(tam,1)*0.0001; %Resistencia no diodo  
RDSON = 0.1 + randn(tam,1)*0.01;      %Resistencia da chave  

Vout = zeros(amostra,tam);          %vetor para as amostras na tensao
IL = zeros(amostra,tam);            %vetor para as amostras na corrente 
%%
for n = 1:tam         %Loop para testes
    f = F(n);
    l = L(n);
    c = C(n);
    d = D(n);  
    rl = RL(n);
    rc = RC(n);
    vd = VD(n);
    rdson = RDSON(n);
    rdiode = RDIODE(n);
    sim('Boost_Converter_SIM');
    Vout(:,n)=vout;
    IL(:,n)=il;
end