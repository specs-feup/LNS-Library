// ============================================================
//  Relatório Final de Estágio — Template Typst
//  Preencha as variáveis abaixo e escreva o conteúdo nas
//  secções correspondentes.
// ============================================================

// ── Metadados do documento ───────────────────────────────────
#let titulo       = "Logarithmic Number System: Approximation Methods and Arithmetic and Logic Unit in a RISC-V Core"
#let autor        = "Henrique dos Santos Teixeira"
#let numero       = "up202306640"          // número de aluno
#let curso        = "Licenciatura de Inteligência Artificial e Ciência de Dados"
#let instituicao  = "INESC TEC"
#let orientador   = "Prof. Eduardo Marques"
#let supervisor   = "Prof. Nuno Paulino"
#let data         = "Maio de 2026"
#let logo_path    = none   // ex: "logo.png" — deixar `none` se não houver

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

// Espaçamento após parágrafos
// #show par: set block(spacing: 0.75em)

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

  // Logo (opcional)
  #if logo_path != none {
    image(logo_path, width: 5cm)
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

  #v(3cm)

  #text(size: 18pt, weight: "bold")[#titulo]

  #v(1cm)

  #text(size: 13pt)[Relatório Final de Estágio]

  #v(3cm)

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

// ── Resumo / Abstract (opcional) ─────────────────────────────
#align(center)[#text(size: 13pt, weight: "bold")[Resumo]]
#v(0.5em)

Escreva aqui um resumo de 150–250 palavras descrevendo o contexto, os
objectivos, as actividades realizadas e as principais conclusões do estágio.

#v(1.5em)
#align(center)[#text(size: 13pt, weight: "bold")[Abstract]]
#v(0.5em)

Write here a 150–250 word abstract in English describing the context,
objectives, activities carried out and main conclusions of the internship.

#pagebreak()

// ── Agradecimentos (opcional) ─────────────────────────────────
#align(center)[#text(size: 13pt, weight: "bold")[Agradecimentos]]
#v(0.5em)

Texto de agradecimentos (opcional).

#pagebreak()

// ── Índice ────────────────────────────────────────────────────
#outline(
  title: [Índice],
  indent: 1.5em,
)

#pagebreak()

// ── Lista de Figuras / Tabelas (opcional) ─────────────────────
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

// ── Abreviaturas / Siglas (opcional) ─────────────────────────
= Abreviaturas e Siglas

#table(
  columns: (auto, 1fr),
  stroke: none,
  inset: (y: 4pt),
  [*API*], [Application Programming Interface],
  [*IST*], [Instituto Superior Técnico],
  // Adicione mais linhas conforme necessário
)

#pagebreak()

// ════════════════════════════════════════════════════════════
//  1. INTRODUÇÃO
// ════════════════════════════════════════════════════════════
= Introdução

Apresente o contexto geral do estágio, a empresa ou instituição de
acolhimento, e a motivação para o trabalho realizado.
Descreva de forma clara os *objectivos* que foram definidos no início do
estágio e a estrutura do presente relatório.

// ════════════════════════════════════════════════════════════
//  2. ENQUADRAMENTO / ESTADO DA ARTE
// ════════════════════════════════════════════════════════════
= Enquadramento e Estado da Arte

Situe o trabalho no contexto científico e tecnológico relevante.
Cite artigos científicos /* @exemplo_artigo */, normas, ferramentas e tecnologias
utilizadas, fornecendo a informação base necessária à compreensão das secções
seguintes.

== Contexto Científico

Descreva os fundamentos teóricos e trabalhos relacionados.

== Ferramentas e Tecnologias

Descreva as principais ferramentas, linguagens e frameworks utilizados.

// ════════════════════════════════════════════════════════════
//  3. DESCRIÇÃO DO TRABALHO
// ════════════════════════════════════════════════════════════
= Descrição do Trabalho

Introdução à secção: descreva brevemente a estrutura das subsecções seguintes.

== Abordagem

Explique a metodologia e as decisões de design tomadas, justificando as
opções adoptadas.

== Implementação

=== Componente / Módulo A

Descreva em detalhe a implementação. Use figuras para ilustrar a arquitectura
ou fluxos importantes.

#figure(
  rect(width: 10cm, height: 5cm, stroke: gray)[
    // Substitua por: image("figura.png", width: 10cm)
    #align(center + horizon)[_[Inserir figura aqui]_]
  ],
  caption: [Diagrama de arquitectura do sistema.],
) <fig_arquitectura>

Como se pode observar na @fig_arquitectura, ...

=== Componente / Módulo B

Continuação da descrição da implementação.

== Resultados

Apresente e analise os resultados obtidos.
Utilize tabelas e figuras sempre que ajudem à clareza.

#figure(
  table(
    columns: (1fr, 1fr, 1fr),
    stroke: 0.5pt,
    inset: 6pt,
    align: center,
    [*Métrica*], [*Valor Obtido*], [*Referência*],
    [Precisão],  [92.3 %],         [90.0 %],
    [Tempo],     [120 ms],         [150 ms],
    // Adicione mais linhas
  ),
  caption: [Resumo dos resultados obtidos.],
) <tab_resultados>

Os resultados da @tab_resultados demonstram que ...

// ════════════════════════════════════════════════════════════
//  4. CONCLUSÃO
// ════════════════════════════════════════════════════════════
= Conclusão

Apresente uma análise crítica do trabalho realizado, sintetizando as
principais conclusões.
Discuta as limitações encontradas e proponha direcções para *trabalho futuro*.

// ════════════════════════════════════════════════════════════
//  BIBLIOGRAFIA
// ════════════════════════════════════════════════════════════
#pagebreak()

// #bibliography("referencias.bib", title: "Bibliografia", style: "ieee")

// Se não tiver ficheiro .bib, substitua pelo bloco manual abaixo:
//
// = Bibliografia
//
// + Apelido, N. (Ano). _Título do artigo_. Revista, vol(n), pp. xx–yy.
// + Apelido, N. (Ano). _Título do livro_. Editora.

// ════════════════════════════════════════════════════════════
//  APÊNDICE (opcional)
// ════════════════════════════════════════════════════════════
#pagebreak()

#set heading(numbering: "A.1  ")
#counter(heading).update(0)

= Apêndice A — Título do Apêndice

Inclua aqui figuras, tabelas, listagens de código ou outros elementos
complementares que não couberam no corpo principal do relatório.

#figure(
  ```python
  def exemplo():
      """Função de exemplo."""
      return 42
  ```,
  caption: [Exemplo de listagem de código.],
)
