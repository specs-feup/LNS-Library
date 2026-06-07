// ============================================================
//  Relatório Final de Estágio — Template Typst
//  Preencha as variáveis abaixo e escreva o conteúdo nas
//  secções correspondentes.
// ============================================================

// ── Metadados do documento ───────────────────────────────────
#let titulo       = "Logarithmic Number System: \nApproximation Methods \n and \nArithmetic and Logic Unit in a RISC-V Core"
#let autor        = "Henrique dos Santos Teixeira"
#let numero       = "up202306640"
#let curso        = "Licenciatura de Inteligência Artificial e Ciência de Dados"
#let instituicao  = "INESC TEC"
#let orientador   = "Prof. Eduardo Marques"
#let supervisor   = "Prof. Nuno Paulino"
#let data         = "Junho de 2026"
#let logo_path    = "fcup.jpg"

// ── Configuração da página ────────────────────────────────────
#set page(
  paper: "a4",
  margin: (top: 2.5cm, bottom: 2.5cm, left: 3cm, right: 2.5cm),
  numbering: "1",
  number-align: center,
)

#set text(
  font: "New Computer Modern",
  size: 11pt,
  lang: "pt",
)

#set heading(numbering: "1.1  ")

#set par(
  justify: true,
  leading: 0.75em,
  first-line-indent: 1.2em,
)

// Estilo dos headings
#show heading.where(level: 1): it => {
  v(1.5em)
  text(size: 14pt, weight: "bold", it)
  v(0.5em)
}
#show heading.where(level: 2): it => {
  v(1em)
  text(size: 12pt, weight: "bold", it)
  v(0.3em)
}
#show heading.where(level: 3): it => {
  v(0.8em)
  text(size: 11pt, weight: "bold", style: "italic", it)
  v(0.2em)
}

// Links
#show link: underline

// ── Capa ─────────────────────────────────────────────────────
#set page(numbering: none)

#align(center)[
  #v(1cm)

  #if logo_path != none {
    image(logo_path, width: 3cm)
    v(0.5cm)
  }

  #text(size: 13pt, weight: "bold")[
    Faculdade de Ciências da
    Universidade do Porto
  ]

  #v(0.4cm)
  #line(length: 100%, stroke: 0.5pt + gray)
  #v(0.4cm)

  #text(size: 11pt, style: "italic")[#curso]

  #v(1cm)

  #text(size: 18pt, weight: "bold")[#titulo]

  #v(1cm)

  #text(size: 13pt)[Relatório Final de Estágio]

  #v(1cm)

  #grid(
    columns: (1fr, 1fr),
    gutter: 1cm,
    align(left)[
      #text(weight: "bold")[Autor] \
      #autor \
      #numero
    ],
    align(left)[
      #text(weight: "bold")[Empresa / Instituição] \
      #instituicao
    ],
  )

  #v(1cm)

  #grid(
    columns: (1fr, 1fr),
    gutter: 1cm,
    align(left)[
      #text(weight: "bold")[Orientador Académico] \
      #orientador
    ],
    align(left)[
      #text(weight: "bold")[Supervisor] \
      #supervisor
    ],
  )

  #v(1fr)

  #line(length: 100%, stroke: 0.5pt + gray)
  #v(0.3cm)
  #text(size: 10pt)[#data]
]

#pagebreak()

// ── Resumo / Abstract ─────────────────────────────────────────
#align(center)[#text(size: 13pt, weight: "bold")[Resumo]]
#v(0.5em)

O Sistema de Numeração Logarítmica (LNS) representa os valores como logaritmos de vírgula fixa em complemento para dois, permitindo que a multiplicação e a divisão sejam realizadas através de simples adições e subtrações inteiras, e que a raiz quadrada se reduza a um deslocamento aritmético à direita. Esta propriedade elimina os multiplicadores de mantissa do hardware, reduzindo significativamente a área e o consumo de energia, características especialmente relevantes para a inferência de redes neuronais em dispositivos de baixos recursos.

Este estágio, realizado no INESC TEC, teve dois objectivos principais. Em primeiro lugar, estender o meu trabalho anterior sobre um núcleo RISC-V baseado em síntese de alto nível (HLS), no qual foi implementada uma Unidade LNS (LNSU) de 16 bits com aproximação por splines lineares, avaliando métodos de aproximação alternativos para as funções de correcção $f^+(t) = log_2(1 + 2^t)$ e $f^-(t) = log_2(1 - 2^t)$. Em segundo lugar, criar uma biblioteca de emulação LNS para suportar inferência em modelos de linguagem de maior escala, nomeadamente a arquitectura Llama 2, e realizar uma comparação sistemática da precisão aritmética entre LNS e bfloat16 em múltiplas bandas de magnitude de operando.

Os principais resultados demonstram que a representação LNS16 Q8.7 supera o bfloat16 em multiplicação e divisão devido à ausência de erros de arredondamento de mantissa, enquanto o bfloat16 mantém vantagem na adição e subtracção. A inferência do modelo de linguagem stories15M com pesos convertidos directamente de FP32 para LNS16 produz texto coerente, confirmando a viabilidade prática do formato para tarefas de inferência de maior complexidade.

#pagebreak()

// ── Agradecimentos ────────────────────────────────────────────
#align(center)[#text(size: 13pt, weight: "bold")[Agradecimentos]]
#v(0.5em)

Agradeço ao meu supervisor Prof. Nuno Paulino pela orientação constante e pelo rigor com que acompanhou este trabalho, bem como aos co-supervisores Luís Sousa e Guilherme Oliveira pelo apoio técnico e pelas discussões produtivas ao longo de todo o estágio. Agradeço também ao INESC TEC pelo ambiente de investigação e pelos recursos disponibilizados.

#pagebreak()

// ── Índice ────────────────────────────────────────────────────
#outline(
  title: [Índice],
  indent: 1.5em,
)

#pagebreak()

// ── Lista de Figuras / Tabelas ────────────────────────────────
#outline(
  title: [Lista de Figuras],
  target: figure.where(kind: image),
)

#v(1em)

#outline(
  title: [Lista de Tabelas],
  target: figure.where(kind: table),
)

#pagebreak()

// ── Início da numeração de páginas ───────────────────────────
#set page(numbering: "1")
#counter(page).update(1)

// ── Abreviaturas / Siglas ─────────────────────────────────────
#align(left)[#text(size: 14pt, weight: "bold")[Abreviaturas e Siglas]]

#table(
  columns: (auto, 1fr),
  stroke: none,
  inset: (y: 4pt),
  [*ALU*],  [Arithmetic Logic Unit],
  [*CPU*],  [Central Processing Unit],
  [*DSP*],  [Digital Signal Processing block],
  [*FF*],   [Flip-Flop],
  [*FP*],   [Floating Point],
  [*FPGA*], [Field-Programmable Gate Array],
  [*FPU*],  [Floating Point Unit],
  [*HLS*],  [High-Level Synthesis],
  [*II*],   [Initiation Interval],
  [*ISA*],  [Instruction Set Architecture],
  [*LNS*],  [Logarithmic Number System],
  [*LNSU*], [Logarithmic Number System Unit],
  [*LUT*],  [Look-Up Table],
  [*MAE*],  [Maximum Absolute Error],
  [*ML*],   [Machine Learning],
  [*MLP*],  [Multi-Layer Perceptron],
  [*MRE*],  [Maximum Relative Error],
  [*NDD*],  [Newton's Divided Differences],
  [*NN*],   [Neural Network],
  [*RISC*], [Reduced Instruction Set Computer],
  [*RMSE*], [Root Mean Square Error],
  [*SoC*],  [System-on-Chip],
  [*XF*],   [Spline format storing (x, f) point pairs],
  [*XMB*],  [Spline format storing (x, m, b) segment coefficients],
)

#pagebreak()

// ════════════════════════════════════════════════════════════
//  1. INTRODUÇÃO
// ════════════════════════════════════════════════════════════
= Introdução

A crescente utilização de redes neuronais em dispositivos de baixos recursos tem intensificado a necessidade de plataformas de computação energeticamente eficientes. Embora o treino de modelos seja frequentemente realizado com aritmética de vírgula flutuante de 32 bits (FP32), a fase de inferência é significativamente mais tolerante à perda de precisão. Como a aritmética IEEE 754 é dispendiosa em termos de hardware e consumo de energia, existe um esforço crescente de investigação orientado para representações numéricas alternativas, incluindo vírgula flutuante de precisão reduzida, vírgula fixa, posits e outros formatos personalizados.

O *Sistema de Numeração Logarítmica* (LNS) constitui uma alternativa particularmente atraente. Ao representar os valores como logaritmos de vírgula fixa em complemento para dois, o LNS transforma a multiplicação e a divisão em adições e subtracções inteiras, e a raiz quadrada num simples deslocamento aritmético à direita. Estas propriedades eliminam os multiplicadores de mantissa do hardware, o que pode reduzir substancialmente a área e o consumo de energia. Adicionalmente, o LNS oferece maior densidade de representação na gama $[-1, 1]$, onde se concentram os pesos típicos de redes neuronais treinadas.

O trabalho realizado neste estágio insere-se num projecto mais amplo desenvolvido no INESC TEC, cujo objectivo é integrar uma extensão LNS no núcleo RISC-V *RISC++* (RISC++), implementado através de HLS. Num trabalho anterior, defini em conjunto com um colega, José Paradela, um formato LNS de 16 bits (1 bit de sinal, 8 bits inteiros, 7 bits fraccionários, designado Q8.7) e implementei uma LNSU com aproximação por splines lineares geradas por um algoritmo guloso, obtendo precisão de 1 bit com tabelas de no máximo 324 bytes. O presente estágio teve por objectivo continuar e aprofundar esse trabalho em duas direcções principais:

+ *Análise de métodos de aproximação alternativos* para as funções de correcção LNS, nomeadamente diferenças divididas de Newton, aproximação minimax (algoritmo de Remez) e aproximação de Chebyshev por troços, comparando-os com as splines lineares existentes em termos de precisão, consumo de memória e custo de avaliação, tanto para os formatos LNS8 e LNS16 existentes como para potenciais formatos LNS32 e LNS64.

+ *Extensão da biblioteca de emulação LNS* para suportar inferência em modelos de linguagem de maior escala baseados na arquitectura Llama 2 (_tiny stories_), incluindo a conversão directa de pesos FP32 para LNS16 e bfloat16, e uma comparação sistemática da precisão aritmética entre os dois formatos em múltiplas bandas de magnitude de operando.

Este relatório está estruturado da seguinte forma: a Secção 2 apresenta o enquadramento científico e tecnológico; a Secção 3 descreve em detalhe o trabalho realizado; e a Secção 4 apresenta as conclusões e perspectivas de trabalho futuro.

// ════════════════════════════════════════════════════════════
//  2. ENQUADRAMENTO / ESTADO DA ARTE
// ════════════════════════════════════════════════════════════
= Enquadramento e Estado da Arte

== O Logarithmic Number System

No LNS, um número real não nulo $x$ é representado por um bit de sinal $S_x$ e um logaritmo binário de vírgula fixa $L_x$ em complemento para dois:

$
S_x = cases(1 "se" x < 0, 0 "se" x >= 0)
quad L_x = log_2(|x|)
quad x = (-1)^(S_x) dot 2^(L_x)
$

O expoente $L_x$ é um número de vírgula fixa em complemento para dois, com uma parte inteira e uma parte fraccionária. Valores negativos de $L_x$ representam magnitudes em $(0, 1)$ e valores positivos representam magnitudes em $(1, +oo)$, o que confere ao LNS maior densidade de representação na gama $[-1, 1]$ — precisamente onde se concentram os pesos típicos de redes neuronais treinadas.

Esta representação transforma várias operações em operações mais simples.

$
z = x dot y quad &=> quad L_z = L_x + L_y \
z = x / y quad   &=> quad L_z = L_x - L_y \
z = sqrt(x) quad &=> quad L_z = L_x >> 1
$

A multiplicação reduz-se a uma adição inteira dos expoentes, a divisão a uma subtracção, e a raiz quadrada a um deslocamento lógico à direita — todos significativamente mais simples e eficientes em termos energéticos do que as suas contrapartes em vírgula flutuante, que requerem manipulação explícita de mantissas.

A adição e a subtracção requerem cuidado adicional. Para $z = x plus.minus y$ com $|x| >= |y|$, a derivação parte de:

$
L_z = log_2(2^(L_x) plus.minus 2^(L_y)) = L_x + log_2(1 plus.minus 2^(L_y - L_x))
$

Definindo $t = L_y - L_x <= 0$ (garantido por troca de operandos se necessário), introduz-se a chamada *função de Gauss logarítmica de adição*:

$
f^+(t) = log_2(1 + 2^t), quad t in (-infinity, 0]
$

e, analogamente para a subtracção, a *função de Gauss logarítmica de subtracção*:

$
f^-(t) = log_2(1 - 2^t), quad t in (-infinity, 0)
$

#pagebreak()

=== Propriedades Analíticas das Funções de Correcção

A função $f^+$ é suave e limitada em $(-oo, 0]$. Cresce monotonicamente de $0$ quando $t -> -oo$ até $1$ em $t = 0$. As suas primeira e segunda derivadas são:

$
(f^+)' (t) = 2^t / (1 + 2^t), quad (f^+)'' (t) = ln(2) dot 2^t / (1 + 2^t)^2
$

Ambas são sempre positivas, monotonicamente crescentes, e aproximam-se de $0$ quando $t -> -oo$ e dos valores $1/2$ e $ln(2)/4$ em $t = 0$. A suavidade de $f^+$ torna-a favorável à aproximação: poucas peças são suficientes para atingir alta precisão.

A função $f^-$ está definida apenas em $(-oo, 0)$. Quando $t -> 0^-$, tem-se $f^-(t) -> -oo$, devido à singularidade do logaritmo em zero. As suas derivadas:

$
(f^-)' (t) = -2^t / (1 - 2^t), quad (f^-)'' (t) = -ln(2) dot 2^t / (1 - 2^t)^2
$

são sempre negativas e divergem para $-oo$ quando $t -> 0^-$. A segunda derivada não limitada perto de $t = 0$ é a razão fundamental pela qual a função de subtracção requer significativamente mais intervalos do que a função de adição para atingir a mesma precisão de aproximação. Qualquer método de aproximação deve tratar esta singularidade explicitamente, quer excluindo uma vizinhança de $0$ e limitando o resultado, quer por uma mudança de variável que regularize o comportamento.

A avaliação eficiente destas funções é o principal desafio de implementação do LNS. As derivações detalhadas das representações em série e dos limites de erro de ordem arbitrária encontram-se em `docs/docs.pdf` no repositório do projecto.

== Formatos Suportados

No meu trabalho anterior, defini com o José dois formatos LNS:

#figure(
  table(
    columns: (auto, auto, auto, auto, auto),
    stroke: 0.5pt,
    inset: 6pt,
    align: center,
    [*Formato*], [*Bit de sinal*], [*Bits inteiros*], [*Bits fraccionários*], [*Gama de valores*],
    [lns8 Q4.3],  [1], [4], [3], [$2^(-8)$ a $2^(7.875)$],
    [lns16 Q8.7], [1], [8], [7], [$2^(-128)$ a $2^(127.992)$],
  ),
  caption: [Especificações dos formatos LNS suportados.],
) <tab_formatos>

O formato lns16 Q8.7 tem 7 bits de mantissa efectiva, o mesmo que o bfloat16, o que torna a comparação directa entre os dois formatos particularmente relevante.

== Trabalho Relacionado

Vários trabalhos recentes exploram formatos aritméticos alternativos em núcleos RISC-V. Em Christ _et al._, é apresentada uma unidade LNS para redes neuronais implementada em FPGA Kintex-7, obtendo uma perda de apenas 3% na precisão top-1 no MNIST relativamente à activação convencional, com 12 500 a 14 000 LUTs. Para posits, trabalhos como o PPU (Wu _et al._) e o PERCIVAL demonstraram que os posits oferecem maior precisão por bit, mas à custa de unidades de hardware que requerem 2.94× mais LUTs e 3.0× mais flip-flops do que uma FPU de 32 bits convencional. Formatos de escala microscópica (MX) como o MXDOTP exploram representações de dois níveis de expoente, obtendo acelerações de até 3.4× face ao FP32 em multiplicações de matrizes. No meu trabalho anterior, demonstrei que a LNSU requer até 45% menos LUTs, 48% menos FFs e 58% menos DSPs do que uma FPU IEEE-754 completa, com frequências de relógio 29%–36% superiores.

== Ferramentas e Tecnologias

O desenvolvimento foi realizado em C++ standard, sem dependências externas além da biblioteca matemática padrão. A biblioteca de emulação LNS (`lnssim.hpp`) e a biblioteca de emulação bfloat16 (`bfloatsim.hpp`) são cabeçalhos auto-contidos que implementam os respectivos tipos numéricos como templates C++. A geração das tabelas de spline é realizada por uma ferramenta autónoma (`spline`) que ajusta aproximações por troços às funções $f^+$ e $f^-$ e escreve os resultados em ficheiros binários `.lns`. A síntese de hardware utiliza AMD Vitis HLS e Vivado 2025.2, com alvo na placa PYNQ-Z2 (Xilinx Zynq-7020). Os modelos de linguagem utilizados nos testes de inferência são da família _TinyStories_ (stories260K e stories15M), baseados na arquitectura Llama 2.

// ════════════════════════════════════════════════════════════
//  3. DESCRIÇÃO DO TRABALHO
// ════════════════════════════════════════════════════════════

#pagebreak()
= Descrição do Trabalho

Este capítulo descreve as actividades realizadas durante o estágio, organizadas em três componentes principais: a análise teórica dos métodos de aproximação para as funções de correcção LNS; a extensão da biblioteca de emulação para suporte a modelos de linguagem; e a realização de uma comparação sistemática de precisão entre LNS e bfloat16.

== Análise de Métodos de Aproximação

=== Propriedades das Funções de Correcção

Um contributo teórico central deste trabalho foi a derivação de limites superiores explícitos para as derivadas de ordem $k$ de ambas as funções de correcção. O ponto de partida é a representação em série infinita de $f^+$ e $f^-$, obtida por expansão em série geométrica de $1/(1 plus.minus 2^t)$ para $t < 0$:

$
log_2(1 plus.minus 2^x) = 1/ln(2)^2 sum_(n >= 1) (minus.plus 1)^n / n dot 2^(n x), quad x < 0
$

Derivando termo a termo (a troca de derivada e soma é justificada pela convergência uniforme), obtém-se a derivada de ordem $k$:

$
d^k / (d x^k) log_2(1 plus.minus 2^x) = ln(2)^(k-2) sum_(n >= 1) (minus.plus 1)^n dot n^(k-1) dot 2^(n x)
$

Para $f^+$, a série é alternada e verifica-se a condição $|a_(n+1)| < |a_n|$ para $n$ suficientemente grande, o que permite aplicar o critério de estimação de erro das séries alternadas e obter um limite finito para a soma da cauda:

$
d^k / (d x^k) log_2(1 + 2^x) <= ln(2)^(k-2) (K + (1 - 2^(x/(k-1)))^(1-k) dot 2^(2x))
$

onde $K$ denota a soma finita dos primeiros termos. Para $f^-$, a série tem todos os termos positivos; aplicando o critério de D'Alembert, a razão entre termos consecutivos converge para $2^x < 1$ (pois $x < 0$), o que permite escrever:

$
d^k / (d x^k) log_2(1 - 2^x) <= ln(2)^(k-2) dot 2^(2x+k-1) / (1 - 2^x)
$

Este limite diverge quando $x arrow.r 0^-$, confirmando analiticamente que a função de subtracção requer significativamente mais intervalos do que a função de adição para atingir a mesma precisão de aproximação — um fenómeno observado empiricamente no trabalho anterior mas que agora dispõe de fundamento teórico rigoroso.

Combinando estes limites com a fórmula de erro da interpolação de Lagrange (Teorema de Lagrange: para $f in C^(n+1)[a,b]$ e $p_n$ o polinómio interpolador em $n+1$ pontos distintos, existe $c_x in [a,b]$ tal que $epsilon = f(x) - p_n(x) = f^((n+1))(c_x) / (n+1)! dot pi_(n+1)(x)$), obtiveram-se as seguintes relações entre a precisão do formato $p$, o grau do polinómio $n_i$, e o tamanho do intervalo $h_i$ em cada segmento:

$
p^+ tilde Omega(log_2((n_i + 1)!) + (n_i + 1) log_2(h_i^(-1)) - 2 x_i)
$

$
p^- tilde Omega(log_2((n_i + 1)!) + (n_i + 1) log_2(h_i^(-1)) - 2 x_i + log_2(1 - 2^(x_i)))
$

O termo extra $log_2(1 - 2^(x_i))$ em $p^-$ é a quantificação exacta do custo adicional imposto pela singularidade perto de $x = 0$. Estas expressões estabelecem um quadro unificado para comparar diferentes métodos de aproximação: dado um grau polinomial $n_i$ e um intervalo $h_i$, é possível estimar a priori a precisão atingível, o que orienta a escolha do método de aproximação para cada formato LNS. As derivações completas encontram-se em `docs/docs.pdf` no repositório do projecto.

=== Domínio Efectivo das Funções

Para um formato LNS com $p$ bits de precisão, o valor mínimo representável em magnitude absoluta é $2^{-p}$. O limite inferior efectivo do domínio de aproximação é portanto:

$x_min^plus.minus = log_2(plus.minus (2^(plus.minus 2^(-p)) - 1))$

Para os formatos considerados, estes limites são:

#figure(
  table(
    columns: (auto, auto, auto, auto),
    stroke: 0.5pt,
    inset: 6pt,
    align: center,
    [*Formato*], [*Precisão $p$ (bits)*], [*$x_min^+$*], [*$x_min^-$*],
    [LNS8 Q4.3],   [3],  [-3.466], [-3.591],
    [LNS16 Q8.7],  [7],  [-7.525], [-7.533],
    [LNS32 Q16.15],[15], [-15.529],[-15.529],
    [LNS64 Q32.31],[30], [-30.529],[-30.529],
  ),
  caption: [Limite inferior efectivo do domínio $x_min^(plus.minus)$ por formato e precisão.],
) <tab_dominios>

Para os formatos de maior precisão (LNS32, LNS64), o domínio cresce proporcionalmente com a largura do expoente. O número de intervalos necessários para manter um limite de erro fixo escala como $O(2^(n/2))$ por unidade de domínio com splines lineares, tornando as abordagens baseadas em tabelas impraticáveis e motivando a transição para representações polinomiais compactas.

=== Funções de Conversão FP ↔ LNS

Uma componente importante do trabalho foi a análise formal das funções de conversão entre FP e LNS. Dado que $x_f_p = (-1)^(S_x) dot (1 + m_x) dot 2^(e_x)$ e $x_"lns" = (-1)^(S_x) dot 2^(L_x)$, a igualdade impõe:

$
L_x = e_x + log_2(1 + m_x)
$

O que define as funções de conversão:

$
"float2lns"(m_x) := log_2(1 + m_x), quad "lns2float"(L_f_x) := 2^(L_f_x) - 1
$

Ambas as funções têm domínio $[0, 1)$ e podem ser aproximadas com splines lineares ou polinómios de Newton. Os limites de erro correspondentes foram derivados analiticamente, seguindo a mesma metodologia aplicada às funções de Gauss logarítmicas:

$
epsilon_"float2lns" <= 1/8 dot 1/(ln(2)) dot 1/((1 + x_(i-1))^2) dot h_i^2
$

$
epsilon_"lns2float" <= 1/8 dot ln(2) dot 2^(2 x_i) dot h_i^2
$

=== Métodos de Aproximação Candidatos

Para além das splines lineares existentes, foram analisados formalmente os seguintes métodos:

*Diferenças Divididas de Newton (NDD).* As diferenças divididas de Newton constroem um polinómio interpolador de grau $n$ através de $n+1$ pontos de amostragem, expresso na forma de Newton:

$P(x) = sum_(i=0)^n [y_0, dots, y_i] product_(j=0)^(i-1) (x - x_j)$

onde $[y_0, dots, y_k]$ denota a diferença dividida de ordem $k$, calculada recursivamente. A avaliação via método de Horner requer $n$ multiplicações e $n$ adições, que é o óptimo. A principal vantagem sobre as splines uniformes é a possibilidade de colocar pontos de amostragem de forma não uniforme, com maior densidade perto de $x = 0$ onde as funções têm maior curvatura, e amostragem mais esparsa na cauda plana perto de $-oo$. Para uma abordagem por troços, o domínio é dividido em segmentos e um polinómio de Newton de grau baixo é ajustado a cada um, com fronteiras de segmento escolhidas adaptativamente em função da curvatura local. O armazenamento de $n+1$ pontos requer $2(n+1)$ valores (os $x_i$ e os coeficientes de diferenças divididas), e para um polinómio de grau $d$ em $k$ segmentos o armazenamento total é $O(k dot d)$ valores.

*Aproximação Minimax e Chebyshev.* A aproximação minimax (algoritmo de Remez) minimiza o erro máximo sobre um intervalo, enquanto a aproximação de Chebyshev por troços utiliza nós de Chebyshev para minimizar o factor $|pi_{n+1}(x)|$ na fórmula de erro de Lagrange. Estes métodos estão identificados como candidatos para trabalho futuro de implementação, especialmente para os formatos LNS32 e LNS64.

== Extensão da Biblioteca de Emulação para Modelos de Linguagem

=== Arquitectura da Biblioteca

A biblioteca de emulação LNS é implementada como dois cabeçalhos C++ auto-contidos: `lnssim.hpp` para emulação em software com tabelas de spline pré-calculadas, e `lns.hpp` para alvo em hardware, que mapeiam os operadores C++ directamente para instruções RISC-V personalizadas via assembly inline. Ambos expõem a mesma interface template `lns<N>`, permitindo que o mesmo código de aplicação seja compilado para qualquer um dos backends por simples substituição do `#include`. A biblioteca pode ser integrada em qualquer projecto com uma única directiva `#include`, sem unidades de compilação adicionais ou dependências externas.

=== Conversão de Modelos FP32 para LNS16

Foi desenvolvida a ferramenta `convert_lns16` (em `src/convert_lns16.cpp`), que converte os ficheiros de pesos de modelos no formato binário Llama 2 de FP32 para LNS16. A conversão opera em modo de streaming em lotes de 4096 pesos de cada vez, mantendo o cabeçalho de configuração do modelo inalterado e convertendo cada peso $x_"fp32"$ para o correspondente $x_"lns16"$ através da relação:

$
L_x = (e_x - 127) + log_2(1 + m_x)
$

onde $e_x$ é o expoente IEEE 754 e $m_x$ é a mantissa normalizada. Esta conversão não requer re-treino do modelo. De forma análoga, foi desenvolvida a ferramenta `convert_bf16` para conversão para bfloat16, o que permite comparações directas entre os dois formatos a partir do mesmo modelo base.

A mesma abordagem de conversão foi aplicada ao tokenizador: os scores dos tokens (armazenados como FP32 no ficheiro binário original) são convertidos para o formato de destino, mantendo os restantes campos intactos.

=== Inferência LNS16 com a Arquitectura Llama 2

O ficheiro `tiny_lns16.cpp` implementa o ciclo completo de inferência do transformador Llama 2 usando exclusivamente aritmética LNS16. Todas as estruturas de dados de activação e pesos (`TransformerWeights`, `RunState`) foram adaptadas para utilizar o tipo `lns16` em lugar de `f32`. As operações fundamentais do transformador — multiplicação de matrizes, atenção multi-cabeça, normalização RMS, SiLU e softmax — foram re-implementadas com o tipo LNS16.

Uma limitação identificada durante a implementação prende-se com a normalização RMS (`rmsnorm`): a soma dos quadrados, sendo uma operação de acumulação que envolve muitas adições sucessivas, é sensível ao erro de aproximação da função de adição LNS, podendo acumular erros que afectam a estabilidade da inferência. Por esta razão, adoptou-se uma abordagem híbrida em que a soma dos quadrados é calculada em FP32 e o resultado convertido para LNS16, mantendo todas as restantes operações em LNS16. Uma abordagem análoga foi adoptada para os cálculos de atenção e softmax, onde as operações de acumulação intensiva foram realizadas em FP32 e os resultados convertidos, enquanto as multiplicações de matrizes — que dominam o custo computacional — foram mantidas em LNS16. Esta estratégia pragmática permite avaliar o impacto do formato LNS nas operações mais relevantes sem incorrer na instabilidade numérica da acumulação puramente logarítmica.

O ficheiro `tiny_lns16_riscpp.cpp` disponibiliza a mesma implementação, mas com as operações mapeadas para as instruções LNS personalizadas via assembly inline, destinando-se à execução no núcleo RISC++ em hardware.

=== Inferência bfloat16

Para fins de comparação, foi igualmente implementada a inferência com bfloat16 (`tiny_bf16.cpp`), seguindo a mesma arquitectura. O tipo `bf16` é implementado em `bfloatsim.hpp` como um tipo de 16 bits que trunca os 16 bits inferiores da representação FP32, operando em FP32 internamente e arredondando para bfloat16 na escrita. Todos os pesos e activações são mantidos em bfloat16, permitindo uma comparação directa com a implementação LNS16 em termos de precisão de inferência e qualidade do texto gerado.

== Comparação Sistemática de Precisão: LNS vs. BFloat16

=== Metodologia

Foi desenvolvido um benchmark abrangente (`main.cpp`) que compara a precisão aritmética de LNS8 vs. BF8 (E4M3) e LNS16 vs. BF16 em cinco operações: round-trip (conversão e retorno), multiplicação, divisão, adição e subtracção. Os testes são parametrizados por bandas de magnitude $["lo", "hi"]$ do valor absoluto de cada operando, todas sendo potências de 2 consecutivas:

- Formatos de 8 bits (lns8, bf8): $[2^{-p+1}, 1]$, $[1, 2]$, $[2, 4]$ (limitado a $|x| <= 4$ para evitar saturação do campo de expoente de 4 bits)
- Formatos de 16 bits (lns16, bf16): $[2^{-p+1}, 1]$, $[1, 2]$, $[2, 4]$, $[4, 8]$, $[8, 16]$, $[16, 32]$, $[32, 64]$, $[64, 2^{16}]$

Dentro de cada banda, os operandos são amostrados log-uniformemente: o expoente é escolhido uniformemente entre as potências de 2 inteiras dentro de $["lo", "hi"]$, uma mantissa aleatória de 23 bits é adicionada, e o sinal é aleatorizado independentemente. A verdade de referência é calculada em FP64.

Uma consideração metodológica importante prende-se com a escolha da métrica primária para adição e subtracção. O erro relativo torna-se arbitrariamente grande quando o resultado se aproxima de zero (cancelamento de operandos), mesmo para formatos correctamente arredondados. Por este motivo, adoptou-se o erro absoluto médio (`avg_abs`) como métrica primária para adição e subtracção, com um filtro de cancelamento adaptado ao formato: pares em que $|a + b| < 2^{-f} dot max(|a|, |b|)$ são descartados, onde $f$ é o número de bits fraccionários do formato em teste ($f = 3$ para 8 bits, $f = 7$ para 16 bits). Esta escolha garante uma comparação equitativa entre LNS e IEEE, pois ambos os formatos são avaliados apenas em entradas onde o resultado é representável com a precisão nativa do formato.

=== Resultados e Análise

Os resultados confirmam as expectativas teóricas. Para multiplicação e divisão, o LNS é consistentemente superior ao bfloat16 em erro relativo médio em todas as bandas: a multiplicação LNS é exacta em termos de expoente (reduz-se a adição inteira), sem erros de arredondamento de mantissa, enquanto o bfloat16 introduz o habitual erro de ~0.5 ULP em cada multiplicação. Para adição e subtracção, o bfloat16 obtém menor erro absoluto médio: a adição IEEE 754 é correctamente arredondada para 1 ULP do resultado, enquanto o erro de adição LNS está ancorado à escala do operando de entrada — limitado pelo orçamento da spline (1 bit) — independentemente de quão pequeno seja o resultado.

Para o round-trip (conversão FP32 → LNS16 → FP32), os resultados são comparáveis entre LNS16 e BF16, o que é esperado: ambos os formatos têm 7 bits de mantissa efectiva, e o pior caso de erro relativo é $approx 2^{-f}$ para ambos na gama utilizável.

#figure(
  table(
    columns: (auto, auto, auto, auto),
    stroke: 0.5pt,
    inset: 6pt,
    align: center,
    [*Operação*], [*Métrica primária*], [*Vencedor (8 bits)*], [*Vencedor (16 bits)*],
    [Round-trip],    [avg\_rel],  [Comparável], [Comparável],
    [Multiplicação], [avg\_rel],  [LNS8],       [LNS16],
    [Divisão],       [avg\_rel],  [LNS8],       [LNS16],
    [Adição],        [*avg\_abs*], [BF8],        [BF16],
    [Subtracção],    [*avg\_abs*], [BF8],        [BF16],
  ),
  caption: [Resumo dos vencedores por operação e largura de bits (baseado em métricas primárias).],
) <tab_benchmark>

// ════════════════════════════════════════════════════════════
//  4. CONCLUSÃO
// ════════════════════════════════════════════════════════════
#pagebreak()
= Conclusão

Este estágio produziu contributos em três áreas complementares. Do ponto de vista teórico, foram derivados limites superiores explícitos para as derivadas de ordem arbitrária de ambas as funções de correcção LNS, estabelecendo um quadro analítico unificado que relaciona a precisão do formato, o grau do polinómio de aproximação e o tamanho dos intervalos. Este quadro fundamenta analiticamente a observação empírica de que a função de subtracção requer substancialmente mais recursos de aproximação do que a de adição.

Do ponto de vista experimental, a comparação sistemática LNS vs. bfloat16 demonstrou que o LNS é superior em multiplicação e divisão (operações dominantes em redes neuronais) enquanto o bfloat16 é superior em adição e subtracção. Esta análise quantitativa por bandas de magnitude fornece uma base mais rigorosa para a avaliação do LNS do que comparações globais de precisão de inferência.

Do ponto de vista de engenharia de software, a extensão da biblioteca de emulação para inferência em modelos de linguagem Llama 2 demonstra a viabilidade do LNS16 em cargas de trabalho mais complexas. A conversão directa de pesos FP32 para LNS16, sem re-treino, produz texto coerente nos modelos _TinyStories_, o que confirma que o LNS16 Q8.7 constitui uma alternativa prática ao bfloat16 para inferência em dispositivos de recursos limitados.

As principais limitações identificadas são a instabilidade numérica nas operações de acumulação intensiva em LNS puro — que motivou a abordagem híbrida LNS/FP32 na normalização RMS e no softmax — e a ausência de resultados de hardware para os formatos LNS32 e LNS64, cujas tabelas de spline são impraticáveis e requerem os métodos de aproximação alternativos identificados neste trabalho.

Como *trabalho futuro*, prevê-se: implementar os métodos de diferenças divididas de Newton e de Chebyshev por troços para os formatos LNS8 e LNS16, comparando-os experimentalmente com as splines lineares existentes; estender a análise para os formatos LNS32 e LNS64 com polinómios compactos; avaliar a LNS em arquitecturas de redes mais profundas e convolucionais; e explorar re-treino ou ajuste fino (_fine-tuning_) com representação LNS para recuperar a precisão perdida na conversão directa.

// ════════════════════════════════════════════════════════════
//  BIBLIOGRAFIA
// ════════════════════════════════════════════════════════════
#pagebreak()

= Bibliografia

+ Parhami, B. (2020). _Logarithmic number systems: Theory and applications_. Advances in Computers, 117, 106800.
+ Coleman, J. N. _et al._ (2000). _Lookup-tables for logarithmic number system operations_. IEEE Trans. Computers, 49(7), 702–715.
+ Christ, M. _et al._ (2022). _Low-precision logarithmic arithmetic for neural network accelerators_. Proc. ASAP 2022.
+ Wu, Y. _et al._ (2025). _PVU: A posit vector processor unit based on RISC-V extension_. arXiv:2503.01313.
+ Islamoglu, E. _et al._ (2025). _MXDOTP: A RISC-V ISA extension for enabling microscaling (MX) floating-point dot products_. arXiv:2505.13159.
+ Rouhani, B. D. _et al._ (2023). _Microscaling data formats for deep learning_. arXiv:2310.10537.
+ Mach, S. _et al._ (2021). _FPnew: An open-source multi-format floating-point unit architecture_. IEEE Trans. VLSI Systems, 29(4), 889–902.
+ Deng, L. (2012). The MNIST database of handwritten digit images for machine learning research. _IEEE Signal Processing Magazine_, 29(6), 141–142.
+ Han, S., Mao, H., & Dally, W. J. (2015). _Deep compression: Compressing deep neural networks with pruning, trained quantization and Huffman coding_. arXiv:1510.00149.
+ Karpathy, A. (2023). _llama2.c_. GitHub. https://github.com/karpathy/llama2.c

// ════════════════════════════════════════════════════════════
//  APÊNDICE
// ════════════════════════════════════════════════════════════
#pagebreak()

#set heading(numbering: "A.1  ")
#counter(heading).update(0)

= Apêndice A — Estrutura do Repositório

O repositório do projecto está organizado da seguinte forma:

#figure(
  ```
  lns/
  ├── lib/
  │   ├── lns/         # lns.hpp  — interface hardware (assembly inline)
  │   ├── lnssim/      # lnssim.hpp — emulação software com tabelas de spline
  │   └── bfloatsim/   # bfloatsim.hpp — emulação bfloat16
  ├── spline/          # Ferramenta de geração de tabelas de spline
  │   └── lns_tables/  # Tabelas pré-calculadas (.lns) para lns8 e lns16
  ├── src/
  │   ├── main.cpp           # Benchmark LNS vs. BFloat16
  │   ├── tiny_lns16.cpp     # Inferência Llama 2 em LNS16 (software)
  │   ├── tiny_lns16_riscpp.cpp # Inferência Llama 2 em LNS16 (hardware)
  │   ├── tiny_bf16.cpp      # Inferência Llama 2 em bfloat16
  │   ├── convert_lns16.cpp  # Conversão de pesos FP32 → LNS16
  │   ├── convert_bf16.cpp   # Conversão de pesos FP32 → bfloat16
  │   └── precision.py       # Cálculo dos limites de domínio
  ├── models/          # Modelos TinyStories (FP32, LNS16, BF16)
  └── docs/            # Documentação e relatórios
  ```,
  caption: [Estrutura de directórios do repositório.],
)

= Apêndice B — Tabelas de Spline LNS16 Q8.7 (XMB)

As tabelas seguintes listam as entradas da tabela de spline XMB utilizadas na LNSU para o formato LNS16 Q8.7. Os valores são representados em hexadecimal no formato de vírgula fixa Q8.7.

#figure(
  table(
    columns: (auto, auto, auto),
    stroke: 0.5pt,
    inset: 6pt,
    align: center,
    [*x (hex)*], [*m (hex)*], [*b (hex)*],
    [`0xfc00`], [`0x0000`], [`0x0000`],
    [`0xfd00`], [`0x0001`], [`0x0008`],
    [`0xfd80`], [`0x0003`], [`0x0014`],
    [`0xfe00`], [`0x0006`], [`0x0023`],
    [`0xfe80`], [`0x000a`], [`0x0033`],
    [`0xfec0`], [`0x0012`], [`0x004b`],
    [`0xff00`], [`0x0016`], [`0x0055`],
    [`0xff40`], [`0x001c`], [`0x0061`],
    [`0xff80`], [`0x0026`], [`0x0070`],
    [`0xffc0`], [`0x0030`], [`0x007a`],
    [`0xffe0`], [`0x0038`], [`0x007e`],
    [`0x0000`], [`0x0040`], [`0x0080`],
  ),
  caption: [Tabela de adição XMB para LNS16 Q8.7 (12 entradas, 72 bytes).],
)
