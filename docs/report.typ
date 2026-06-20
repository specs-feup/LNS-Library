// ============================================================
//  Relatório Final de Estágio — Template Typst
//  Preencha as variáveis abaixo e escreva o conteúdo nas
//  secções correspondentes.
// ============================================================

// ── Metadados do documento ───────────────────────────────────
#let titulo       = "Evaluating Logarithmic Number Systems for Low-Resource Language Models"
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
  margin: (top: 2cm, bottom: 2cm, left: 3cm, right: 2.5cm),
  numbering: "1",
  number-align: center,
)

#set text(
  font: "New Computer Modern",
  size: 11pt,
  lang: "pt",
  region: "PT"
)

#set heading(numbering: "1.1  ")
#set cite(style: "ieee")

#set par(
  justify: true,
  leading: 0.65em,
  first-line-indent: 1.2em,
)

// Estilo dos headings
#show heading.where(level: 1): it => {
  v(1em)
  text(size: 14pt, weight: "bold", it)
  v(0.3em)
}
#show heading.where(level: 2): it => {
  v(0.6em)
  text(size: 12pt, weight: "bold", it)
  v(0.2em)
}
#show heading.where(level: 3): it => {
  v(0.5em)
  text(size: 11pt, weight: "bold", style: "italic", it)
  v(0.15em)
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

  #text(size: 20pt, weight: "bold")[#titulo]

  #v(1cm)
  #link("https://github.com/specs-feup/LNS-Library")[github.com/specs-feup/LNS-Library]

  #v(1fr)

  #text(weight: "bold")[Autor] \
  #autor \

  #line(length: 100%, stroke: 0.5pt + gray)
  #v(0.3cm)
  #text(size: 10pt)[#data]
]

#pagebreak()

// ── Resumo / Abstract ─────────────────────────────────────────
#align(center)[#text(size: 13pt, weight: "bold")[Resumo]]
#v(0.5em)

O Logarithmic Number System (LNS) representa os valores como
logaritmos de vírgula fixa em complemento para dois,
permitindo que a multiplicação e a divisão sejam realizadas
através de simples adições e subtrações inteiras e que a
raiz quadrada se reduza a um deslocamento aritmético à
direita. Esta propriedade elimina os multiplicadores de
mantissa do hardware, reduzindo significativamente a área
e o consumo de energia, características especialmente
relevantes para a inferência de redes neuronais em
dispositivos de baixos recursos.

Este estágio, realizado no INESC TEC, teve quatro objectivos
principais: em primeiro lugar, a continuação de trabalho
realizado previamente com o LNS, que foi focado nas
operações aritméticas, ao adicionar operações de
comparação e conversão de Floaint Point/Brain Float (FP/BF) 
para LNS e vice-versa,
em segundo lugar, criar uma biblioteca de
emulação LNS para criar uma base de exploração do formato,
em terceiro lugar, realizar uma comparação
estatística da precisão aritmética
entre LNS e BF em múltiplos intervalos 
e, em quarto lugar, suportar inferência
em LLMs, nomeadamente a arquitectura Llama 2.

No regime de 8 bits, o `bf8` é o claro vencedor, no entanto, 
o `lns8` tem apenas uma pequena desvantagem sob o `bf8`
na multiplicação e divisão em todos os intervalos 
testados, enquanto tem uma maior vantagem na adição,
subtracção e round-trip (conversão de um número em
`fp32` para o formato e conversão de volta para `fp32`) 
— padrão que reflecte
directamente a natureza do LNS, onde a multiplicação e a
divisão são operações exactas no domínio logarítmico, ao
passo que a adição e a subtracção requerem funções de
correção não-lineares sujeitas a erro de aproximação.
No regime de 16 bits, o `bf16` domina o erro relativo na
multiplicação, adição e subtracção, enquanto o `lns16`
mantém vantagem consistente na divisão em todos os
intervalos e no erro absoluto da multiplicação. 
Nos testes numéricos, ambos os formatos de 8 bits
têm um erro relativo equivalente abaixo de 1.0 em
tarefas de acumulação intensiva como uma progressão
geométrica e uma série harmónica alternada,
evidenciando as limitações inerentes à precisão reduzida,
equanto que em 16 bits, o LNS apresenta um erro
significantemente inferior ao BF. Também no regime
de 16 bits, o `lns16` apresenta erro superior ao `bf16` em
funções como a GELU, cuja avaliação envolve composição
de operações não-lineares pouco favoráveis ao domínio
logarítmico, no entanto, o `lns16` apresenta um menor 
erro na Softmax face ao `bf16`. 
A inferência do modelo stories15M e stories42M
com pesos convertidos directamente de `fp32` para `lns16`
produz texto coerente, confirmando a viabilidade prática
do formato para tarefas de inferência de maior complexidade.

#v(1.5em)
#align(center)[#text(size: 13pt, weight: "bold")[Agradecimentos]]
#v(0.5em)

Agradeço ao meu supervisor Prof. Nuno Paulino pela 
orientação constante e pelo rigor com que acompanhou 
este trabalho, bem como aos co-supervisores Luís Sousa 
e Guilherme Oliveira pelo apoio técnico e pelas 
discussões produtivas ao longo de todo o estágio. 
Agradeço também ao INESC TEC pelo ambiente de 
investigação e pelos recursos disponibilizados.

#pagebreak()

// ── Índice ────────────────────────────────────────────────────
#outline(
  title: [Índice],
  indent: 1.5em,
)

#pagebreak()

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
  [*BF*],   [Brain Float],
  [*CPU*],  [Central Processing Unit],
  [*DSP*],  [Digital Signal Processing block],
  [*FP*],   [Floating Point],
  [*FPGA*], [Field-Programmable Gate Array],
  [*FPU*],  [Floating Point Unit],
  [*HLS*],  [High-Level Synthesis],
  [*LLM*],  [Large Language Model],
  [*LNS*],  [Logarithmic Number System],
  [*LNSU*], [Logarithmic Number System Unit],
  [*LUT*],  [Look-Up Table],
  [*MAE*],  [Mean Absolute Error],
  [*MRE*],  [Mean Relative Error],
  [*RISC*], [Reduced Instruction Set Computer],
  [*XF*],   [Spline format storing $(x, f)$ point pairs],
  [*XMB*],  [Spline format storing $(x, m, b)$ segment coefficients],
)

#pagebreak()

// ════════════════════════════════════════════════════════════
//  1. INTRODUÇÃO
// ════════════════════════════════════════════════════════════
= Introdução

A crescente utilização de redes neuronais em dispositivos
de baixos recursos tem intensificado a necessidade de
plataformas de computação energeticamente eficientes.
Embora o treino de modelos seja frequentemente realizado
com aritmética de Floating Point (FP) de 32 bits (`fp32`),
a fase de inferência é significativamente mais tolerante
à perda de precisão. Como a aritmética IEEE 754 é
dispendiosa em termos de hardware e consumo de energia,
existe um esforço crescente de investigação orientado para
representações numéricas alternativas, incluindo 
Brain Float (BF),
Low-Precision Floating Point, vírgula fixa, posits e
outros formatos personalizados.

O *Logarithmic Number System* (LNS) constitui uma
alternativa particularmente atraente. Ao representar
os valores como logaritmos de vírgula fixa em complemento
para dois, o LNS transforma a multiplicação e a divisão
em adições e subtracções inteiras e a raiz quadrada num
simples deslocamento aritmético à direita. Estas
propriedades eliminam os multiplicadores de mantissa do
hardware, o que pode reduzir substancialmente a área e
o consumo de energia. Adicionalmente, o LNS oferece
maior densidade de representação na gama $[-1, 1]$,
onde se concentram os pesos típicos de redes neuronais
treinadas.

O trabalho realizado neste estágio insere-se num projecto
mais amplo desenvolvido no INESC TEC, cujo objectivo é a
continuação do desenvolvimento
de uma extensão LNS num CPU RISC-V (RISC++),
implementado através de High Level Synthesis (HLS),
um processo de design automatizado que converte software
abstrato (como C, C++ ou SystemC) em circuitos digitais
de hardware (Register-Transfer Level — RTL).
Num trabalho anterior, defini em conjunto com o meu colega,
José Paradela, dois formatos LNS, `lns16 Q8.7` e `lns8 Q4.3`, e implementei
uma LNSU com as operações
`add`, `sub`, `mul`, `div` e `sqrt` e na qual
as funções não-lineares intrínsecas à
adição e subtração eram aproximadas por
splines lineares gerados
por um algoritmo greedy, obtendo precisão de $2^(-7)$
e $2^(-3)$, respetivamente,
com tabelas de no máximo 324 bytes. O presente estágio
teve por objectivo continuar e aprofundar esse
trabalho em quatro direcções principais:

+ #link(<comp-and-conv>)[*adicionar operações de comparação e conversão*] de FP/BF $<->$ LNS;

+ #link(<lns-lib>)[*criar uma biblioteca de emulação LNS*] para criar uma base de exploração do formato;

+ #link(<comparacao-lns-bfloat>)[*comparar o LNS ao BF*] em precisão aritmética com testes estatísticos;

+ #link(<llm-inf>)[*suportar inferência em LLMs*], nomeadamente a arquitectura Llama 2.

Este relatório está estruturado da seguinte forma:
- a @enquadramento-soa apresenta o estado da arte e o enquadramento;
- a @work-description descreve em detalhe o trabalho realizado;
- a @sec-resultados reúne os resultados e procede à análise destes;
- a @conclusion-and-future-work expõe as conclusões e perspectivas de trabalho futuro.

// ════════════════════════════════════════════════════════════
//  2. ENQUADRAMENTO / ESTADO DA ARTE
// ════════════════════════════════════════════════════════════
#pagebreak()
= Estado da Arte e Enquadramento <enquadramento-soa>

== Estado da Arte

#heading(level: 3, outlined: false)[O Logarithmic Number System]

No LNS, um número real não nulo $x$ é representado por
um bit de sinal $S_x$ e um logaritmo binário de vírgula
fixa $L_x$ em complemento para dois:

$
S_x = bb(1)[x < 0]
quad quad L_x = log_2(|x|)
quad quad x = (-1)^(S_x) dot 2^(L_x)
$

O expoente $L_x$ é um número de vírgula fixa em complemento
para dois, com uma parte inteira e uma parte fraccionária.
Valores negativos de $L_x$ representam magnitudes em $(0, 1)$
e valores positivos representam magnitudes em $(1, +oo)$,
o que confere ao LNS maior densidade de representação na
gama $[-1, 1]$ — precisamente onde se concentram os pesos
típicos de redes neuronais treinadas.

Segue-se a tabela com a transformação das operações em
LNS:

#align(center)[
  #set text(size: 12pt)
  #table(
    columns: (0.4fr, 0.5fr, 1fr, 0.6fr),
    inset: 0.4em,
    stroke: 0.3pt + luma(180),
    fill: (x, y) => if y == 0 { rgb("#8cBdDa") } else { luma(248) },
    
    table.header(
      [*$z$*], [*$S_z$*], [*$L_z$*], [*Condição*]
    ),
    [$x + y$],      [$S_x$],          [$L_x + log_2(1 plus 2^(L_y - L_x))$],   [$forall x, y, abs(x) > abs(y)$],
    [$x - y$],      [$S_x$],          [$L_x + log_2(1 minus 2^(L_y - L_x))$],  [$forall x, y, abs(x) > abs(y)$],
    [$x dot y$],    [$S_x xor S_y$],  [$L_x + L_y$],                           [$forall x, y$],
    [$x div y$],      [$S_x xor S_y$],  [$L_x - L_y$],                           [$forall x, y$],
    [$x^(2^n)$],    [$0$],            [$L_x << n$],                            [$forall n > 1$],
    [$x^(2^(-n))$], [$0$],            [$L_x >> n$],                            [$forall n > 1, x >= 0$],
  )
]

A multiplicação reduz-se a uma adição inteira dos expoentes,
a divisão a uma subtração e as potências ou raízes de base 2 a meros
deslocamentos lógicos à esquerda ou à direita (shifts) — todos significativamente
mais simples e eficientes em termos energéticos do que
as suas contrapartes em vírgula flutuante, que requerem
manipulação explícita de mantissas. Parhami @parhami2020 constitui
a referência canónica do LNS, descrevendo-o como uma alternativa
estruturalmente distinta à vírgula flutuante: ao representar os valores
como logaritmos de vírgula fixa em complemento para dois, o sistema
transforma a multiplicação em adição inteira e a divisão em subtracção
inteira.

A adição e a subtracção requerem cuidado adicional.
Para $z = x plus.minus y$ com $|x| >= |y|$,
a derivação parte de:

$
z = x plus.minus y => L_z &= log_2(|x plus.minus y|) \
&= log_2(|2^(L_x) plus.minus 2^(L_y)|) \
&= log_2(2^(L_x) dot |1 plus.minus 2^(L_y - L_x)|) \
&= L_x + log_2(|1 plus.minus 2^(L_y - L_x)|) \
&= L_x + log_2(1 plus.minus 2^(L_y - L_x)) \
$

#pagebreak()

Definindo $t = L_y - L_x <= 0$ (garantido por troca 
de operandos se necessário), introduz-se a chamada 
*função de Gauss logarítmica de adição*:

$
f^+(t) = log_2(1 + 2^t), quad t in (-infinity, 0]
$

e, analogamente para a subtracção, a *função de Gauss 
logarítmica de subtracção*:

$
f^-(t) = log_2(1 - 2^t), quad t in (-infinity, 0)
$

A função $f^+$ é suave e limitada em $(-oo, 0]$.
Cresce monotonicamente de $0$ quando $t -> -oo$ até $1$
em $t = 0$. As suas primeira e segunda derivadas são:

$
(dif)/(dif t) f^+(t) = 2^t / (1 + 2^t), quad (dif^2)/(dif t^2) f^+(t) = ln(2) dot 2^t / (1 + 2^t)^2
$

Ambas são sempre positivas, monotonicamente crescentes,
e aproximam-se de $0$ quando $t -> -oo$ e dos valores
$1/2$ e $ln(2)/4$ em $t = 0$. A suavidade de $f^+$
torna-a favorável à aproximação: poucas peças são
suficientes para atingir alta precisão.

A função $f^-$ está definida apenas em $(-oo, 0)$.
Quando $t -> 0^-$, tem-se $f^-(t) -> -oo$, devido à
singularidade do logaritmo em zero. As suas derivadas:

$
(dif)/(dif t) f^-(t) = -2^t / (1 - 2^t), quad (dif^2)/(dif^2 t) f^-(t) = -ln(2) dot 2^t / (1 - 2^t)^2
$

são sempre negativas e divergem para $-infinity$
quando $t -> 0^-$. A segunda derivada não limitada
perto de $t = 0$ é a razão fundamental pela qual a
função de subtracção requer significativamente mais
intervalos do que a função de adição para atingir a
mesma precisão de aproximação — assimetria confirmada
analiticamente e documentada por Coleman et al. @coleman2000,
que demonstram que $f^-$ requer significativamente mais
entradas de LUT do que $f^+$ devido à singularidade em
$t = 0$, explorando ainda compromissos entre dimensão das
tabelas e precisão, o que motiva directamente a representação
XMB adoptada. O autor cataloga as principais dificuldades
— nomeadamente a complexidade das funções de correção $f^+$
e $f^-$ — e revê estratégias para a sua avaliação eficiente.
A avaliação eficiente destas funções é o principal desafio de
implementação do LNS. As derivações detalhadas encontram-se em
`docs/docs.pdf` no repositório do projecto.

#pagebreak()

#heading(level: 3, outlined: false)[LNS em Aceleração de Redes Neuronais]

Christ et al. @christ2022 apresentam uma unidade LNS de baixa
precisão para aceleradores de redes neuronais em FPGA Kintex-7,
obtendo degradação de apenas 3% na precisão top-1 no MNIST com
12 500 a 14 000 LUTs. A arquitectura partilha o objectivo de
minimizar o custo das funções de correção via LUTs de dimensão
reduzida. A diferença central é de escala: enquanto Christ et al.
@christ2022 focam redes convolucionais compactas, o presente
trabalho estende a análise para inferência em modelos Llama 2
(stories15M e stories42M), com cargas de trabalho
substancialmente mais intensas.

#heading(level: 3, outlined: false)[Formatos Alternativos: Posits e Microscaling]

Wu et al. @wu2025 apresentam o PPU, uma extensão RISC-V para
aritmética posit vectorial. Os posits oferecem maior precisão
efectiva por bit em certas gamas, mas a custos de hardware
consideráveis: o PERCIVAL requer 2,94× mais LUTs e 3,0× mais
flip-flops do que uma FPU convencional. Em contraste, a LNSU
do trabalho anterior requer até 45% menos LUTs, 48% menos
flip-flops e 58% menos DSPs, com frequências 29%–36% superiores.
Os formatos de microscaling (MX) @rouhani2023 @islamoglu2025
adoptam abordagem diferente: introduzem um expoente partilhado
por blocos de elementos, reduzindo o custo de representação sem
alterar a aritmética. O MXDOTP @islamoglu2025 obtém acelerações
de até 3,4× face ao `fp32` em multiplicações de matrizes. Estes
formatos são ortogonais ao LNS e poderiam, em princípio,
ser combinados.

#heading(level: 3, outlined: false)[Compressão e Quantização de LLMs]

Han et al. @han2015 demonstraram que redes neuronais profundas
toleram quantização agressiva dos pesos sem degradação
significativa, desde que a estrutura do modelo seja preservada —
motivação directa para a conversão directa `fp32` → `lns16` sem
re-treino adoptada neste trabalho. A arquitectura Llama 2 nos
modelos _TinyStories_ @karpathy2023 utiliza multi-head attention
e normalização RMS, operações cujo comportamento numérico em
formatos de baixa precisão é menos estudado do que o das redes
convolucionais clássicas. A produção de texto coerente após
conversão directa confirma que o `lns16 Q8.7` é uma alternativa
prática ao `bf16` para inferência em dispositivos de recursos
limitados.

#pagebreak()

== Enquadramento

#heading(level: 3, outlined: false)[Representações XF e XMB]

O trabalho anterior aproximou $f^+$ e $f^-$ com
piecewise splines lineares gerados por um algoritmo greedy.
Dado um conjunto de $n+1$ pontos $\{(x_k, f_k)\}_{k=0}^{n}$,
a fórmula da interpolação de um spline linear para
um intervalo $[x_(k-1),x_k]$ é:

$ "LS"_k (x) = ((x_k - x) f_(k-1) + (x - x_(k-1)) f_k) / h_k, quad h_k = x_k - x_(k-1) $ <eq:xf>

Isto é a representação XF. Uma forma equivalente é:

$ "LS"_k (x) = m_k dot.c x + b_k, quad m_k = (f_k - f_(k-1))/h_k, quad b_k = (f_(k-1) x_k - f_k x_(k-1))/h_k $ <eq:xmb>

Esta é a representação XMB. Precomputa o declive $m_k$ e a
interceção $b_k$ por intervalo, reduzindo a avaliação em
runtime a um simples multiply-add. O trade-off é o
aumento no uso da memória: XF requere $4 dot (n + 1)$ bytes
para o `lns16`, enquanto o XMB requere $6 dot (n + 1)$ bytes.

A fórmula para o upper bound do error de um spline linear é:

$
epsilon <= 1/8 max_[a,b] |f''(x)| h_i^2
$

#heading(level: 3, outlined: false)[Formatos Suportados]

#figure(
  table(
    columns: (auto, auto, auto, auto, auto),
    stroke: 0.3pt + luma(180),
    inset: 6pt,
    align: center,
    fill: (x, y) => if y == 0 { rgb("#8cBdDa") } else { luma(248) },

    table.header(
      [*Formato*], [*$S_x$ (Sinal)*], [*$L_x_i$ (Inteira)*], [*$L_x_f$ (Fracionária)*], [*Intervalo*],
    ),
    [`lns8 Q4.3`],  [1], [4], [3], [$2^(-8)$ a $2^(7.875)$],
    [`lns16 Q8.7`], [1], [8], [7], [$2^(-128)$ a $2^(127.992)$],
  ),
  caption: [Especificações dos formatos LNS suportados.],
) <tab_formatos>

O formato `lns16 Q8.7` tem 7 bits de mantissa efectiva,
o mesmo que o `bf16`, tornando a comparação directa
entre os dois formatos particularmente relevante.

#heading(level: 3, outlined: false)[Erros dos Splines de Aproximação]

A fórmula dos erros para o spline de aproximação das
funcões $f^+$ e $f^-$ são:

$
epsilon_k^+ <= (ln(2) dot.c 2^(x_k) dot.c h_k^2) / (8(1 plus 2^(x_k))^2)

quad quad

epsilon_k^- <= (ln(2) dot.c 2^(x_k) dot.c h_k^2) / (8(1 minus 2^(x_k))^2)
$

Como a upper bound é monótona e crescente para ambas as
funções, o intervalo a separar em dois é sempre aquele com maior
$h_k$ ponderado pela curvatura local. O mínimo tamanho de
intervalo permitido é $2 dot 2^(-f)$, onde $f$ é a parte
fracionária do formato LNS.

#pagebreak()

#heading(level: 3, outlined: false)[Algoritmo Greedy]

O algoritmo greedy começa com as duas extremidades do
intervalo inicial $[-8, 0)$. 
Iterativamente adiciona o ponto intermédio do intervalo
com maior estimativa de erro. Tanto para `lns8 Q4.3` 
como para `lns16 Q8.7`, $f^(plus.minus)(-8)$ é um 
número que ambos os formatos não conseguem
representar.

#figure(
  align(left)[
    #set text(font: "Liberation Mono", size: 9pt)
    #block(
      width: 100%,
      stroke: 0.5pt + luma(150),
      inset: 10pt,
      radius: 4pt,
      fill: luma(250),
    )[
      #set par(justify: false)
      *Algorithm* GreedySplineSelection($X, Y, M, B, S, f, epsilon$) \
      *Inputs*: Empty sequence $X$, Empty sequence $Y$,
      Empty sequence $M$, Empty sequence $B$,
      target size $S$,
      target function $f$, error metric $epsilon$ \
      *Output*: Optimized spline table $T$
      #line(length: 100%, stroke: 0.3pt + gray)

      #let ind(n) = h(n * 1em)
      #let comment(c) = h(5pt) + text(fill: gray, style: "italic")[▷ #c]

      #set enum(numbering: "1.", indent: 0pt)

      #enum.item(1)[$X <- ["start", "end"]$]
      #enum.item(2)[*while* $|X| < S$ *do*]
      #enum.item(3)[#ind(1) *for* $j <- 1$ *to* $|X| - 1$ *do*]
      #enum.item(4)[#ind(2) $h_j <- X_(j+1) - X_j$]
      #enum.item(5)[#ind(2) $e_j <- epsilon(X_j, h_j)$]
      #enum.item(6)[#ind(1) *end for*]
      #enum.item(7)[#ind(1) $"max_j" <- arg max_j (e_j) " where " h_j > 2 dot 2^(-f)$]
      #enum.item(8)[#ind(1) $X_("new") <- X_"max_j" + (h_"max_j" >> 1)$]
      #enum.item(9)[#ind(1) $X_("max_j"+1) <- x_("new")$]
      #enum.item(10)[*end while*]
      #v(0.4em)
      #enum.item(11)[*for* $i <- 1$ *to* $|X|$ *do*]
      #enum.item(12)[#ind(1) $Y_i <- f(X_i)$ #comment[valor da ordenada $y$ do ponto do formato XF]]
      #enum.item(13)[*end for*]
      #v(0.4em)
      #enum.item(14)[*for* $i <- 2$ *to* $|X|$ *do*]
      #enum.item(15)[#ind(1) $k <- floor(log_2(X_i - X_(i-1)))$]
      #enum.item(16)[#ind(1) $M_i <- (X_i - X_(i-1)) >> k$ #comment[valor do declive $m$ do formato XMB]]
      #enum.item(17)[#ind(1) $B_i <- Y_i - ((M_i dot.c X_i) >> k)$ #comment[valor da ordenada na origem $b$ do formato XMB]]
      #enum.item(18)[*end for*]
    ]
  ],
  caption: [Pseudocódigo do Algoritmo Greedy para Tabelas Spline],
) <alg-greedy-spline-clean>

#heading(level: 3, outlined: false)[Ferramentas e Tecnologias]

O desenvolvimento foi realizado em C++ standard,
sem dependências externas além da biblioteca
matemática padrão. A biblioteca de emulação
LNS (`lnssim.hpp`) e a biblioteca de emulação
BF (`bfloatsim.hpp`) são cabeçalhos
auto-contidos que implementam os respectivos
tipos numéricos como templates C++.
A geração das tabelas de spline é realizada
por uma ferramenta autónoma (`spline`) que ajusta
aproximações por troços às funções $f^+$ e $f^-$ e
escreve os resultados em ficheiros binários `.lns`.
Os modelos de linguagem utilizados nos testes de
inferência são da família _TinyStories_ @karpathy2023
(stories15M e stories42M), baseados na arquitectura
Llama 2.


// ════════════════════════════════════════════════════════════
//  3. DESCRIÇÃO DO TRABALHO
// ════════════════════════════════════════════════════════════

#pagebreak()
= Descrição do Trabalho <work-description>

Esta secção apresenta uma descrição detalhada do
trabalho desenvolvido no decurrer do estágio, o qual
teve como objetivo principal dar continuidade e
aprofundar o ecossistema computacional planeado,
focando-se em quatro partes fundamentais.
Inicialmente, o trabalho centrou-se em
#link(<comp-and-conv>)[adicionar operações de
comparação e conversão] entre os formatos FP/BF e
LNS. De forma a estabelecer uma base prática
de exploração do formato, procedeu-se à
#link(<lns-lib>)[criação de uma biblioteca de
emulação LNS]. Subsequentemente, realizou-se uma
análise rigorosa para
#link(<comparacao-lns-bfloat>)[comparar o LNS ao BF]
em termos de precisão aritmética recorrendo a
testes estatísticos. Por fim, este ecossistema foi
validado ao #link(<llm-inf>)[suportar inferência em
LLMs], focando-se especificamente na arquitetura
Llama 2.

== Comparações em LNS e Conversão FP/BF $<->$ LNS <comp-and-conv>

#heading(level: 3, outlined: false)[Comparações em LNS]

Sejam $x$ e $y$ dois operandos, podemos estabelecer as
seguintes 5 comparações:

$
x < y, quad x <= y, quad x = y, quad x >= y, quad x > y
$

com apenas duas delas:

$
x < y " e " x = y
$

Para determinar se $x$ é igual a $y$, basta comparar os bits
um a um, logo, é equivalente a fazer uma comparação de
inteiros.

Para determinar se $x$ é menor do que $y$:

$
"LNS-LessThan"(x, y) = cases(
  "true"                        & "se" "sign"(x) = 1 and "sign"(y) = 0,
  "exponent"(x) > "exponent"(y) & "se" "sign"(x) = "sign"(y) = 1,
  "exponent"(x) < "exponent"(y) & "se" "sign"(x) = "sign"(y) = 0,
  "false"                       & "caso contrário"
)
$

As restantes comparações são estabelecidas através de
rescritas das inequações:
$
x <= y &<=> x < y or x = y quad quad
x >= y &<=> y <= x quad quad
x > y &<=> y < x
$

#heading(level: 3, outlined: false)[Funções de Conversão FP/BF ↔ LNS]

Uma componente importante do trabalho foi a análise formal
das funções de conversão entre FP e LNS.
Dado que $x_"fp" = (-1)^(S_x) dot (1 + m_x) dot 2^(e_x - 127)$
e $x_"lns" = (-1)^(S_x) dot 2^(L_x)$, a igualdade impõe
$L_x = e_x - 127 + log_2(1 + m_x)$,
o que define as funções de conversão:

$
"float2lns"(S_x, e_x, m_x) := (-1)^(S_x) dot (e_x - 127 + log_2(1 + m_x)) \
"lns2float"(S_x, L_x_i, L_x_f) := (-1)^(S_x) dot (1 + (2^(L_x_f) - 1)) dot 2^(L_x_i + 127)
$

Ambas as expressões não lineares das funções de conversão, 
$log_2(1 + m_x)$ e $2^(L_x_f) - 1$, 
têm domínio $[0, 1)$ e podem ser
aproximadas com splines lineares. Os upper bounds do erro
correspondentes foram derivados analiticamente, seguindo
a mesma metodologia aplicada às funções de Gauss
logarítmicas:

$
epsilon_"float2lns" <= 1/8 dot 1/(ln(2)) dot 1/((1 + x_(i-1))^2) dot h_i^2
quad quad quad
epsilon_"lns2float" <= 1/8 dot ln(2) dot 2^(2 x_i) dot h_i^2
$

// ═══════════════════════════════════════════════════════════════
//  SECÇÃO 3.2 — Biblioteca de Emulação LNS
// ═══════════════════════════════════════════════════════════════

== Biblioteca de Emulação LNS <lns-lib>

#heading(level: 3, outlined: false)[Arquitectura da Biblioteca]

A biblioteca de emulação LNS é implementada como um conjunto de
cabeçalhos C++ auto-contidos que partilham a mesma interface template
`lns<N, I, F>`, parametrizada pela largura total em bits (`N`), pelo
número de bits inteiros (`I`) e pelo número de bits fraccionários (`F`).
Esta interface expõe os operadores aritméticos C++ standard (`+`, `-`,
`*`, `/`, comparações, etc.) e permite que o mesmo código de aplicação seja
compilado para dois backends distintos por simples substituição do
`#include`:

- `lnssim.hpp` — emulação em software com tabelas de spline
  pré-calculadas (formato `.lns`), destinado a simulação e testes no
  host. A aritmética de adição e subtracção é resolvida por pesquisa na
  tabela XF ou XMB, dependendo de qual foi carregada via
  `lns{8,16}_read_tables()`. Todas as restantes operações são exactas no
  domínio logarítmico.

- `lns.hpp` — interface para alvo em hardware que mapeia cada operador
  directamente para a instrução RISC-V personalizada correspondente via
  assembly inline, destinado à execução no núcleo RISC++ em FPGA.

A biblioteca também inclui `bfloatsim.hpp`, um cabeçalho auto-contido
que implementa os tipos `bf8` (E4M3) e `bf16` (E7M8) 
como templates
C++. O `bf16` é simulado truncando os 16 bits inferiores de uma
representação `fp32` (round-to-zero); o `bf8` aplica a mesma estratégia
com a máscara E4M3. Ambos os tipos expõem a mesma interface que os tipos
LNS correspondentes, permitindo que o mesmo benchmark seja instanciado
sobre qualquer combinação de formatos.

Os tipos concretos utilizados ao longo deste trabalho são definidos como:

```cpp
typedef lns<32, 8, 23> lns32;   // acumulador de alta precisão
typedef lns<16, 8,  7> lns16;   // formato principal Q8.7
typedef lns< 8, 4,  3> lns8;    // formato de 8 bits Q4.3
```

`lns32` é utilizado como referência de verdade: os erros de `lns8` e
`lns16` são calculados relativamente a `lns32`, enquanto os erros de
`bf8` e `bf16` são calculados relativamente a `fp32` e `f64`,
respectivamente. Esta assimetria é intencional — cada formato é julgado
contra uma versão de maior precisão de si próprio.

A biblioteca pode ser integrada em qualquer projecto C++ com uma única
directiva `#include`, sem dependências externas e sem sistema de build
específico. 

Todas as interfaces mencionadas encontram-se no #link(<anexo-apis>)[Anexo].

A geração das tabelas usadas pelos formatos de `lns` de $8$ e $16$
bits é feita por uma custom tool chamada `spline`. Esta gera-as
para ficheiros binários `.lns` e permite a customização do diretório
em que os escreve. A descrição do formato do ficheiro das tabelas e
exemplo de utilização da ferramenta para geração encontra-se
no #link(<anexo-spline>)[Anexo].


// ═══════════════════════════════════════════════════════════════
//  SECÇÃO 3.3 — Comparação de Precisão Aritmética: LNS vs. BF
// ═══════════════════════════════════════════════════════════════

#pagebreak()
== Comparação de Precisão Aritmética: LNS vs. BF <comparacao-lns-bfloat>

#heading(level: 3, outlined: false)[Metodologia]

Foi desenvolvido um benchmark abrangente (`examples/bench/`) que compara
a precisão aritmética de LNS e BF nos pares `lns8`/`bf8` e `lns16`/`bf16`,
cobrindo duas dimensões: precisão por intervalos em quatro
operações aritméticas e round-trip (Benchmark 1) e precisão em nove 
kernels (Benchmark 2).

#heading(level: 4, outlined: false, numbering: none)[Benchmark 1 — Operações Aritméticas e Round-Trip]

As cinco operações avaliadas são: conversão e retorno (_round-trip_),
multiplicação, divisão, adição e subtracção, parametrizadas por intervalos $["lo", "hi"]$ (potências de dois consecutivas). 

A métrica primária é diferenciada por operação: erro relativo médio (MRE)
para round-trip, multiplicação e divisão (invariante à
escala); erro absoluto médio (MAE) para adição e subtracção, onde
o erro relativo diverge em situações de cancelamento. Aplica-se um filtro
de cancelamento: pares em que
$|a + b| < 2^(-f) dot max(|a|, |b|)$
são descartados, onde $f$ é o número de bits fraccionários.

Os vencedores são determinados por teste Mann-Whitney U bilateral com
$n = 100,000$ amostras ($p < 0.01$, $|r| >= 0.05$).

#heading(level: 4, outlined: false, numbering: none)[Benchmark 2 — Kernels]

Nove testes de nível algorítmico: progressão geométrica, norma
euclidiana, série harmónica alternada (Leibniz), acumulação da
soma de $pi^2/6$ com a definição da série do Basel Problem tanto
do primeiro até ao termo $N$ e o reverso, sigmoid, GELU, soma de 
softmax e RMSNorm. LNS é avaliado contra lns32; BF contra f32.
Para cada teste em formato de 16 bits, é testada soma acumulada
em 32 bits, `lns16` testado com `lns32` e `fp32`, enquanto que
o `bf16` tem soma acumulada apenas com `fp32`.
Empates declarados quando a diferença relativa entre erros é 
inferior a 5%.


#pagebreak()
== Inferência na Arquitectura Llama 2 <llm-inf>

#heading(level: 3, outlined: false)[Conversão de Modelos `fp32` para `lns16` e `bf16`]

Foram desenvolvidas duas ferramentas de conversão de pesos 
em
`examples/tinystories/convert/`: `convert_lns16.cpp` e 
`convert_bf16.cpp`.
Ambas operam em modo de streaming em lotes de 4096 
pesos de cada vez,
mantendo o cabeçalho de configuração inalterado 
e convertendo apenas
os pesos e os scores do tokenizer.

A conversão `fp32` → `lns16` aplica, para cada peso $x_"fp32"$:

$
L_x = (e_x - 127) + log_2(1 + m_x)
$

onde $e_x$ é o expoente IEEE 754 e $m_x$ é a mantissa 
normalizada em
$[0, 1)$. O termo $log_2(1 + m_x)$ é avaliado via 
a tabela `float2lns`
pré-calculada pelo algoritmo greedy. Esta conversão não 
requer re-treino.
A conversão `fp32` → `bf16` trunca os 16 bits inferiores 
da representação
`fp32`, mantendo sinal, 8 bits de expoente e 7 bits 
de mantissa.

#heading(level: 3, outlined: false)[Inferência `lns16` vs `bf16`]

O ficheiro `examples/tinystories/tiny/tiny_lns16.cpp` implementa o 
ciclo
completo de inferência do transformer Llama 2 usando 
exclusivamente
`lns16` para pesos e activações. As operações fundamentais 
foram
re-implementadas com o tipo `lns16`: multiplicação de 
matrizes
(_matmul_), atenção multi-cabeça, normalização RMS, 
SiLU e softmax.

A normalização RMS é sensível ao erro de aproximação 
da adição LNS, 
uma vez que a soma dos quadrados envolve muitas adições 
sucessivas. Adoptou-se
por isso uma abordagem híbrida em que essa 
soma é calculada em `fp32` e
convertida para `lns16`, mantendo todas as restantes 
operações em LNS16.
Uma estratégia análoga foi aplicada às operações de 
acumulação intensiva
em atenção e softmax. Esta escolha permite avaliar o 
impacto do LNS
nas operações de maior custo computacional — as 
multiplicações de
matrizes — sem incorrer na instabilidade numérica da 
acumulação
puramente logarítmica.

Um processo equivalente ocorreu com o ficheiro `examples/tinystories/tiny/tiny_lns16_lns32acc.cpp`
em que, em vez de usar `fp32` para a acumulação da soma, usou-se a simulação
de `lns32` (drop-in replacement).
Foi também estado todos os modelos com `bf16` com acumulação da soma em `fp32`
(ficheiro `examples/tinystories/tiny/tiny_bf16.cpp`).

É de realçar que, sem a acumulação da soma com `fp32`/`lns32` em `bf16` e `lns16`, os modelos não 
conseguiam dar output lógico de texto.

#heading(level: 3, outlined: false)[Inferência `lns8` vs `bf8`]

Inferência com ambos os formatos de 8 bits foi testada, mas
o output do modelo era ou a repetição do mesmo token ou
acabava numa divisão por zero, caso do `lns8`. A precisão 
destes formatos é muito baixa e, como os modelos são relativamente
pequenos, afeta bastante a computação.

#pagebreak()

= Resultados e Análise <sec-resultados>

Esta secção apresenta e discute os resultados obtidos
nos testes experimentais, estruturando-se em dois
eixos principais de avaliação. Primeiro, expõe-se uma
análise comparativa detalhada da precisão aritmética
entre os formatos LNS e BF. Segundo, demonstra-se a viabilidade
prática destes formatos através da sua aplicação direta
na inferência de modelos de linguagem da arquitetura
Llama 2.

== Comparação de Precisão Aritmética: LNS vs. BF

#align(center)[
  #heading(level: 4, outlined: false, numbering: none)[Benchmark 1 — Operações Aritméticas e Round-Trip]
]

#v(10pt)

#figure(
  image("results/ops_errors.png", width: 100%),
  caption: [Erro relativo e absoluto médio por intervalo para cada operação e par de
    formatos. \ ($p$ é o número de bits da parte inteira do exponte em LNS — 
    $2^(-p + 1)$ é o número mais pequeno que ambos os formatos conseguem 
    representar)],
) <fig_ops_errors>

#pagebreak()

O erro relativo é aproximadamente invariante ao intervalo 
para todos os formatos; a diferença LNS/BF é 
essencialmente constante ao longo dos intervalos. 

O erro absoluto cresce com a magnitude para
todos os formatos. Os gráficos de adição, subtracção,
multiplicação e round-trip mostram o erro absoluto de 
ambos os formatos a crescer,
consistente com a acumulação de erro na 
correção de adição em log-space.

#figure(
  image("results/ops_heatmap_combined_rel_abs.png", width: 100%),
  caption: [Comparação de heatmaps de vencedor por operação × intervalo para os formatos de 8 bits (`lns8` vs `bf8`). À esquerda: erro relativo médio; À direita: erro absoluto médio.],
) <fig_heatmaps>

Em erro relativo e absoluto, o `bf8` vence em todos os três intervalos sem empates. 
O resultado de multiplicação e divisão é digno de nota: embora a aritmética LNS 
seja exata no expoente e o `bf8` sofra com apenas 3 bits de mantissa, 
este permanece mais preciso. Adicionalmente, 
o resultado de adição e subtracção confirma a fraqueza conhecida do LNS, onde o 
termo de correção aproximado com a tabela introduz um erro que a adição directa 
do `bf8` não sofre.

Em erro absoluto, o `lns16` vence a multiplicação em 7 dos 8 intervalos e a
divisão em todos os 8; o `bf16` vence adição e subtracção em todos os
intervalos. Em erro relativo, o `bf16` inverte o resultado na multiplicação.
Esta inversão é significativa: o `lns16` é preferível quando os operandos
permanecem numa gama limitada e a fidelidade absoluta importa; o `bf16`
é preferível quando a precisão relativa tem de ser mantida num intervalo
dinâmico amplo.



#align(center)[
  #heading(level: 4, outlined: false, numbering: none)[Benchmark 2 — Kernels]
]

#v(10pt)

#figure(
  image("results/numerical_rel.png", width: 100%),
  caption: [Erro relativo médio dos testes numéricos para todos os formatos.
    Os empates são anotados quando a diferença relativa entre lns e bf é
    inferior a 5%.],
) <fig_numerical_rel>

#figure(
  image("results/numerical_abs.png", width: 100%),
  caption: [Erro absoluto médio dos testes numéricos para todos os formatos.
    Os empates são anotados quando a diferença relativa entre lns e bf é
    inferior a 5%.],
) <fig_numerical_abs>

Os resultados numéricos são consistentes com a avaliação
das operações por intervalo: em qualquer workload dominado
pela subtração ou adição, a adição corretamente
arredondada do `bf16` dá-lhe uma vantagem visível. As
variantes de acumulação em 32 bits mostram que esta
diferença é praticamente devida à precisão na acumulação
em vez do formato: `lns16_lns32_acc` e `lns16_f32acc`
recuperam bastante em testes como `pi2_over6`, 
`softmax_sum` e `rmsnorm_denom`. Em variantes de
puras sequências de multiplicação as variantes de 16 bits
em LNS têm melhor precisão do que BF.
Nos formatos de 8 bits, à uma maior competitividade, o `lns8`
mostra-se por vezes melhor que o `bf8` ou equivalente. É de
realçar também, que em `lns8`, as versões de acumulação em
32 bits nem sempre melhoram o erro devido ao erro
constante de conversão entre formatos.

#pagebreak()

== Inferência na Arquitetura Llama 2

Todos os modelos foram corridos com os parâmetros
default exceto o tokenizer que muda de acordo com
o formato LNS/BF.

#set text(size: 10pt)
#align(center)[
  #table(
    columns: 3,
    stroke: 0.3pt + luma(180),
    align: center,
    fill: (x, y) => if y == 0 { rgb("#8cBdDa") } else { luma(248) },

    table.header(
      [*Parâmetro*], [*Descrição*], [*Valor Default*],
    ),
    [Temperature], 
    [Controla creatividade; $0.0$ é determinístico, $1.0$ é creativo],
    [$1.0$],
    [Top-p],
    [Nucleus sampling; remove $1 - "topp"$ dos tokens menor prováveis],
    [$1.0$],
    [Steps],
    [Máximo número de tokens da geração],
    [$256$],
    [Prompt],
    [Texto inicial que o modelo usa como baseline para começar],
    [NULL],
    [rng_seed],
    [Semente do gerador de números aleatórios; 0 usa system time],
    [$0$],
    [Mode],
    [Determina como o modelo se comporta (generate/chat)],
    [generate],
    [System Prompt],
    [Usado apenas em modo chat para maior instruction context],
    [NULL],
  )
]

#heading(level: 4, outlined: false, numbering: none)[Stories 15M]

#v(10pt)

- `lns16` (XF)

#rect(
  width: 100%,
  inset: 8pt,
  stroke: 0.5pt + gray,
  radius: 4pt,
)[
  _Once upon a time, there was a little girl named Lily. She loved to draw and color with her crayons. One day, she drew a beautiful picture of a rainbow with lots of colors.
Her mom saw the picture and said, "Lily, that's a pretty flower. You should try to make it even more beautiful."
Lily smiled and said, "I want to design a new picture of a rainbow!"
Her mom gave her some paper and crayons and Lily started to draw. But then she got confused and drew the same big, bright colors. She said, "Mommy, this picture is not perfect."
Her mom said, "Oh no, I'm sorry. We can't print it. But let's draw another one."
Lily learned that sometimes things don't go the way we want them to, but it's important to keep trying and not give up._
]

- `lns16` (XMB)

#rect(
  width: 100%,
  inset: 8pt,
  stroke: 0.5pt + gray,
  radius: 4pt,
)[
  _One day, a boy named Tim and a girl named Sue went to the beach. They saw a lot of water and a big, pretty sun. They wanted to play and splash in the water. Tim had a toy boat that he would push in the water. Sue had a ball that she liked to throw.
Tim said, "Let's race our boats!" Sue agreed and they put their boats in the water. Tim's boat went very fast, and Sue's ball went in the water. They laughed and clapped their hands as they watched the boats swim.
After a while, Tim felt tired. He said, "I'm hungry. Let's stop and eat." Sue agreed and they started to eat their lunch. They sat on the sand and watched the boats in the water. They had a fun day at the beach and went home happy._
]

- `bf16`

#rect(
  width: 100%,
  inset: 8pt,
  stroke: 0.5pt + gray,
  radius: 4pt,
)[
  _Once upon a time, there was a little girl named Lily. She had a big brother named Max who was very dependable. Max always helped Lily when she needed it. One day, Lily wanted to play with her toys but Max said no. Lily was sad and angry. She wanted to play with Max's toys.
Lily's mom noticed her daughter was upset and asked her what was wrong. Lily explained that Max wouldn't let her play with his toys. Lily's mom talked to Max and told him that Lily was being mean. Max didn't listen and kept saying mean things. Lily felt better and decided to talk to Max. She told him that she didn't want to play with him and that it was okay to be mean. Max realized he was being mean and gave Lily a toy to play with. They both played together and had fun._
]

#pagebreak()

#heading(level: 4, outlined: false, numbering: none)[Stories 42M]

#v(10pt)

- `lns16` (XF)

#rect(
  width: 100%,
  inset: 8pt,
  stroke: 0.5pt + gray,
  radius: 4pt,
)[
  _Once upon a time, there was a little boy named Tim. Tim was very excited because he was going on a holiday with his family. They were going to the beach! Tim loved the beach because he could play in the sand and swim in the sea.
One day, Tim's family decided to have a picnic in the park. They packed sandwiches and fruit to eat. But when they went to the beach, they saw that the sand was not there! They looked everywhere but couldn't find it. Tim was sad because he really wanted to eat the sandwich.
Suddenly, they heard a loud noise and saw a big wave coming towards them. They were scared because it was a storm. But then, the wave brought a big box with them. They opened the box and found a treasure! It was a big toy boat that Tim and his family could use. They were so happy and thanked the wave for bringing the treasure._
]

- `lns16` (XMB)

#rect(
  width: 100%,
  inset: 8pt,
  stroke: 0.5pt + gray,
  radius: 4pt,
)[
  _Once upon a time, there was a little girl named Lily. She loved to help her mom in the kitchen. One day, her mom asked her to get some ice from the freezer. Lily was so happy to help! She ran to the freezer and opened it. Inside, she found a rare ice cream.
Lily was so excited to eat it, but then she saw something she didn't expect. The ice cream started to melt in her hands! She quickly took it off and ran to her mom. "Mommy, the ice cream is melting! It's making me panic!" she exclaimed.
Her mom laughed and said, "Don't worry, Lily. It's just a melted ice cream. Let's go back to the kitchen and get more." Lily was happy to get more ice cream, but she knew she had to wait for it to melt away._
]

- `bf16`

#rect(
  width: 100%,
  inset: 8pt,
  stroke: 0.5pt + gray,
  radius: 4pt,
)[
  _Once upon a time, there was a little boy named Tim. Tim was a pupil in a small school. He had a friend named Sam. Sam was a very reliable friend. They liked to play and laugh together.
One day, Tim and Sam went to play outside. They saw a big tree and wanted to climb it. Tim said, "Let's go up, Sam!" Sam agreed, and they started to climb the tree. As they climbed, they saw a bird in the tree. The bird said, "Hello, Tim and Sam!"
Tim and Sam were very surprised. They did not know that birds could talk. The bird said, "I am a magic bird. If you help me, I will give you a surprise." Tim and Sam thought about it and said, "Yes, we can help you!"
The magic bird told Tim and Sam to close their eyes. When they opened them, they were now in a magical world. They met many animals and had many fun adventures. They were very happy and thankful for the magic bird and his help._
]


#heading(level: 3, outlined: false)[Análise]

A produção de texto coerente pelos modelos _TinyStories_ 
após conversão
directa `fp32` → `lns16`, sem qualquer re-treino, 
confirma a viabilidade
prática do `lns16 Q8.7` para inferência em redes neuronais. 

// ════════════════════════════════════════════════════════════
//  4. CONCLUSÃO
// ════════════════════════════════════════════════════════════
#pagebreak()
= Conclusão e Trabalho Futuro <conclusion-and-future-work>

Este estágio aprofundou e alargou uma implementação LNS prévia em quatro
direcções: operações de comparação e conversão, biblioteca de emulação,
comparação estatística com BF e inferência em modelos de linguagem.
Os resultados permitem tirar conclusões claras sobre os pontos fortes e
as limitações do formato.

O LNS é inequivocamente superior em multiplicação e divisão — as
operações dominantes em redes neuronais. No regime de 8 bits, o `bf8`
vence o `lns8` em todas as operações em todos os intervalos testados,
embora na multiplicação e divisão estas diferenças sejam menores; no
regime de 16 bits, o `lns16` mantém vantagem consistente em divisão e em
erro absoluto de multiplicação. A razão é estrutural: multiplicação e
divisão são exactas no domínio logarítmico, reduzindo-se a operações
inteiras sem erro de arredondamento de mantissa. Em contrapartida, a
adição e a subtracção requerem a avaliação das funções de Gauss
logarítmicas $f^+$ e $f^-$ por tabela de consulta, introduzindo erro
de aproximação que o IEEE 754 não sofre. Esta assimetria é o traço
definidor do LNS e deve informar qualquer decisão de adopção: o formato
é mais vantajoso em cargas de trabalho dominadas por produtos de
matrizes, e menos adequado a pipelines com acumulação intensiva.

A viabilidade do `lns16 Q8.7` para inferência foi confirmada empiricamente.
A conversão directa de pesos `fp32` para `lns16`, sem re-treino, produz texto
coerente nos modelos stories15M e stories42M, corroborando que o erro
introduzido nas multiplicações de matrizes é suficientemente pequeno para
preservar a qualidade da saída. A abordagem híbrida adoptada — acumulação
em `fp32`/`lns32`, multiplicações em `lns16` — é um compromisso pragmático que
delimita claramente onde o LNS contribui valor e onde a sua fraqueza
aritmética obrigaria a mitigação adicional.

Como *trabalho futuro*, os eixos mais imediatos são: implementar métodos
de aproximação de maior ordem — diferenças divididas de Newton e
Chebyshev por troços — para reduzir o número de intervalos necessários
em $f^-$ e tornar praticáveis os formatos lns32 e lns64; avaliar o LNS
em arquitecturas convolucionais e em modelos de linguagem de maior
escala; e explorar re-treino ou ajuste fino com representação LNS para
recuperar a precisão perdida na conversão directa, potencialmente
eliminando a necessidade da abordagem híbrida nas operações de
acumulação.

// ════════════════════════════════════════════════════════════
//  BIBLIOGRAFIA
// ════════════════════════════════════════════════════════════
#pagebreak()

#bibliography("bibliography.bib", style: "ieee")

// ════════════════════════════════════════════════════════════
//  APÊNDICE
// ════════════════════════════════════════════════════════════

#pagebreak()

#set heading(numbering: "A.1  ")
#counter(heading).update(0)

= Anexo

== APIs <anexo-apis>

#heading(level: 3, outlined: false, numbering: none)[`#include <lns>`]

```cpp
template<u8 n>
struct lns {
  static constexpr u8 f_width =
    n == 8  ? 3 :
    n == 16 ? 7 :
    n == 32 ? 19 : 47;

  f32 bits;

  lns();
  lns(u32 raw);

  explicit operator f32() const;

  lns  operator+ (const lns other) const;
  lns  operator- (const lns other) const;
  lns  operator- ()                const;
  lns  operator* (const lns other) const;
  lns  operator/ (const lns other) const;

  lns  sqrt      ()                const;

  lns& operator+=(const lns other);
  lns& operator-=(const lns other);
  lns& operator*=(const lns other);
  lns& operator/=(const lns other);

  void operator= (const lns other) volatile;

  bool operator==(const lns other) const;
  bool operator!=(const lns other) const;
  bool operator< (const lns other) const;
  bool operator<=(const lns other) const;
  bool operator> (const lns other) const;
  bool operator>=(const lns other) const;

  static inline lns  load (const void* addr, u32 imm = 0);
  inline        void store(void* addr, u32 imm = 0) const;
};

using lns8  = lns<8>;
using lns16 = lns<16>;
using lns32 = lns<32>;
using lns64 = lns<64>;
```

#pagebreak()

#heading(level: 3, outlined: false, numbering: none)[`#include <lnssim>`]

```cpp
template<u8 n, u8 i, u8 f>
struct lns {
  uint_t<n> bits;

  lns();
  lns(uint_t<n> raw, bool);
  lns(i32 x);
  lns(f32 x);
  template<u8 n2, u8 i2, u8 f2>
  lns(const lns<n2, i2, f2>& other);

  explicit operator f32() const;
  explicit operator f64() const;

  u8       sign()     const;
  int_t<n> exponent() const;
  bool     is_zero()  const;

  lns operator+   (const lns other)   const;
  lns operator-   (const lns other)   const;
  lns operator-   ()                  const;
  lns operator*   (const lns other)   const;
  lns operator/   (const lns other)   const;

  lns power2_pow  (const u8 k)        const;
  lns power2_root (const u8 k)        const;

  lns square      ()                  const;
  lns sqrt        ()                  const;
  lns exp         ()                  const;
  lns sinh        ()                  const;
  lns cosh        ()                  const;
  lns tanh        ()                  const;

  // ... (restantes operadores de atribuição e comparação)
};
```

#pagebreak()

#heading(level: 3, outlined: false, numbering: none)[`#include <bfloatsim>`]

```cpp
struct bf8 {
  u8 bits; // E4M3: [S | EEEE | MMM]

  bf8();
  bf8(u8 raw, bool);
  bf8(i32 x);
  bf8(bf16 x);
  bf8(f32 x);

  operator f32() const;

  inline bool operator== (const bf8 other) const;
  inline bool operator<  (const bf8 other) const;
  inline bool operator>  (const bf8 other) const;
  inline bool operator<= (const bf8 other) const;
  inline bool operator>= (const bf8 other) const; 

  bf8 operator+          (const bf8 other) const; 
  bf8 operator-          (const bf8 other) const; 
  bf8 operator-          ()                const; 
  bf8 operator*          (const bf8 other) const; 
  bf8 operator/          (const bf8 other) const; 

  // ... (restantes operadores de atribuição e comparação)
};

struct bf16 {
  u16 bits; // E8M7: [S | EEEEEEEE | MMMMMMM]

  bf16();
  bf16(u16 raw, bool);
  bf16(i32 x);
  bf16(bf8 x);
  bf16(f32 x);

  inline operator f32() const;

  inline bool operator== (const bf16 other) const;
  inline bool operator<  (const bf16 other) const;
  inline bool operator>  (const bf16 other) const;
  inline bool operator<= (const bf16 other) const;
  inline bool operator>= (const bf16 other) const; 

  bf16 operator+          (const bf16 other) const; 
  bf16 operator-          (const bf16 other) const; 
  bf16 operator-          ()                const; 
  bf16 operator*          (const bf16 other) const; 
  bf16 operator/          (const bf16 other) const; 

  // ... (restantes operadores de atribuição e comparação)
};
```

#pagebreak()

== Spline <anexo-spline>

Os ficheiros `.lns` guardam os dados de interpolação em formato binário bruto (_raw binary_) em modo _Little-Endian_, prontos a serem mapeados diretamente para a memória do simulador.

#heading(level: 3, outlined: false, numbering: none)[Cabeçalho Fixo (16 bytes)]
O ficheiro começa sempre com quatro inteiros de 32 bits sem sinal (`u32`) que definem o número de linhas (nós) de cada sub-tabela:

#align(center)[
  #set text(size: 10pt)
  #table(
    columns: (1fr, 1fr, 1fr, 1fr),
    inset: 0.8em,
    stroke: 0.5pt + luma(150),
    fill: (x, y) => if y == 0 { luma(220) } else { luma(250) },
    table.header([*Bytes 0 a 3*], [*Bytes 4 a 7*], [*Bytes 8 a 11*], [*Bytes 12 a 15*]),
    [`s_plus` (u32)], [`s_minus` (u32)], [`s_f2l` (u32)], [`s_l2f` (u32)],
    [Tam. Adição], [Tam. Subtração], [Tam. FP $arrow$ LNS], [Tam. LNS $arrow$ FP]
  )
]

#heading(level: 3, outlined: false, numbering: none)[Layout do Payload (Dados Sequenciais)]

Após o cabeçalho, os dados das tabelas surgem de forma contígua na seguinte ordem:
#align(center)[
  #highlight(fill: rgb("e0f2fe"))[`spline_plus`] $arrow$ 
  #highlight(fill: rgb("fee2e2"))[`spline_minus`] $arrow$ 
  #highlight(fill: rgb("fef08a"))[`spline_f2l`] $arrow$ 
  #highlight(fill: rgb("bbf7d0"))[`spline_l2f`]
]

O formato e o tamanho de cada registo dentro das tabelas dependem da precisão (`LNS16`/`LNS8`) e do modo de cálculo (`XF`/`XMB`):

#align(center)[
  *Modo XF* (Interpolação por Coordenadas): *2/4 bytes por linha*:
  #table(
    columns: (2cm, 2cm),
    inset: 0.5em,
    stroke: 0.5pt + luma(100),
    fill: luma(245),
    [`x` (i8/i16)], [`y` (i8/i16)]
  )
]

#align(center)[
  *Modo XMB* (Declive e Translação): *3/6 bytes por linha*:
  #table(
    columns: (2cm, 2cm, 2cm),
    inset: 0.5em,
    stroke: 0.5pt + luma(100),
    fill: luma(245),
    [`x` (i8/i16)], [`m` (i8/i16)], [`b` (i8/i16)]
  )
]

Para o formato de 8 bits, os dados sofrem um _bit-shift_ à direita antes de serem gravados, reduzindo o consumo de memória para metade.

Nas tabelas `f2l` e `l2f` do formato `LNS8`, a operação binária aplicada aos dados originais de 16-bit antes da escrita é: `byte = (i16_val >> 8) & 0xFF`.

#heading(level: 3, outlined: false, numbering: none)[Geração e Customização de Tabelas (`spline`)]

A ferramenta CLI em `lib/spline/` gera e testa as tabelas binárias `.lns`. O binário suporta dois modos de operação via _flags_ estritas:

```bash
# Compilação e geração automática dos perfis padrão (Q8.7 e Q4.3)
cd lib/spline && make

```

#heading(level: 3, outlined: false, numbering: none)[Sintaxe da CLI]

`bash ./build/spline <--gen | --test> [config] `

- `--gen`: Exporta os ficheiros `.lns`.
  - requer 4 valores: `[+, -, f2l, l2f]` (linhas: $[2, 1024]$).
- `--test`: Print do erro de aproximação no `stdout`.
  - requer 1 valor: `[max_linhas]` ($[2, 1024]$).

- `[config]`
  - `--xf` / `--xmb`: pontos vs. declive ($m, b$).
  - `--lns16` / `--lns8`: largura do formato.
  - `<int_digits>`: bits inteiros; intervalo de valores possíveis em `lns8` é $[4,6]$ e em `lns16` é $[4,14]$.

#heading(level: 4, outlined: false, numbering: none)[ Exemplos Práticos]

#v(10pt)

*A. Geração Personalizada (`--gen`)*

Cria tabelas em modo *XMB* para *lns16* ($"Q"8.7$) 
com tamanhos assimétricos por sub-tabela:

```bash
# Ordem: + - f2l l2f | int: 8
./build/spline --gen --xmb 128 256 64 64 --lns16 8
```

*B. Teste de Precisão (`--test`)*

Avalia o impacto do número de linhas no erro para *lns8* 
($"Q"4.3$) em modo *XF* (sem gravar ficheiros):

```bash
# Varrimento até 128 linhas | int: 4
./build/spline --test --xf 128 --lns8 4
```

*Nota:* O modo `--test` mapeia o par 
`Erro vs Linhas` para validação rápida 
antes da síntese em FPGA.

#v(10pt)

#heading(level: 4, outlined: false, numbering: none)[Exemplo de Utilização das Tabelas]

#v(10pt)

```cpp
#define SPLINE_XMB
#include <lnssim.hpp>

using lns8  = lns<8,  4, 3>;
using lns16 = lns<16, 8, 7>;

lns8_read_tables ("lib/spline/lns_tables/lns8_q4_3_xmb.lns");
lns16_read_tables("lib/spline/lns_tables/lns16_q8_7_xmb.lns");

lns16 a(1.5f), b(2.0f);
float result  = (float)(a * b);   // exact — integer add on exponents
float result2 = (float)(a + b);   // approximated via spline LUT
float result3 = (float)a.sqrt();  // exact — exp >>= 1

lns_close();
```
