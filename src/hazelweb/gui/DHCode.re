module Vdom = Virtual_dom.Vdom;
open Pretty;

let view_of_layout = (~inject, l: DHLayout.t): Vdom.Node.t => {
  open Vdom;
  let rec go = (l: DHLayout.t) =>
    switch (l) {
    | Text(s) =>
      if (List.length(String.split_on_char('\\', s)) <= 1) {
        [Node.text(s)];
      } else if (String.length(s) == 1) {
        print_endline("UHCode87");
        [Node.text(s)];
      } else {
        switch (String.sub(s, 0, 1)) {
        | "\\" =>
          switch (String.sub(s, 1, 1)) {
          | "b"
          | "t"
          | "r"
          | "n"
          | "\""
          | "\'" => [
              Node.text(String.sub(s, 0, 2)),
              ...go(Text(String.sub(s, 2, String.length(s) - 2))),
            ]
          | "\\" => [
              Node.text("\\"),
              ...go(Text(String.sub(s, 2, String.length(s) - 2))),
            ]
          | "o" =>
            if (String.length(s) >= 5) {
              let ch1 = s.[2];
              let ch2 = s.[3];
              let ch3 = s.[4];
              if ((ch1 >= '0' && ch1 <= '7')
                  && (ch2 >= '0' && ch2 <= '7')
                  && ch3 >= '0'
                  && ch3 <= '7') {
                if (ch1 <= '3') {
                  [
                    Node.text(String.sub(s, 0, 5)),
                    ...go(Text(String.sub(s, 5, String.length(s) - 5))),
                  ];
                } else {
                  [
                    Node.span(
                      [Attr.classes(["InvalidSeq"])],
                      [Node.text(String.sub(s, 0, 5))],
                    ),
                    ...go(Text(String.sub(s, 5, String.length(s) - 5))),
                  ];
                };
              } else {
                [
                  Node.span(
                    [Attr.classes(["InvalidSeq"])],
                    [Node.text(String.sub(s, 0, 2))],
                  ),
                  ...go(Text(String.sub(s, 2, String.length(s) - 2))),
                ];
              };
            } else {
              [
                Node.span(
                  [Attr.classes(["InvalidSeq"])],
                  [Node.text(String.sub(s, 0, 2))],
                ),
                ...go(Text(String.sub(s, 2, String.length(s) - 2))),
              ];
            }
          | "x" =>
            if (String.length(s) >= 4) {
              let ch1 = Char.lowercase_ascii(s.[2]);
              let ch2 = Char.lowercase_ascii(s.[3]);
              if ((ch1 >= '0' && ch1 <= '9' || ch1 >= 'a' && ch1 <= 'f')
                  && (ch2 >= '0' && ch2 <= '9' || ch2 >= 'a' && ch2 <= 'f')) {
                [
                  Node.text(String.sub(s, 0, 4)),
                  ...go(Text(String.sub(s, 4, String.length(s) - 4))),
                ];
              } else {
                [
                  Node.span(
                    [Attr.classes(["InvalidSeq"])],
                    [Node.text(String.sub(s, 0, 2))],
                  ),
                  ...go(Text(String.sub(s, 2, String.length(s) - 2))),
                ];
              };
            } else {
              [
                Node.span(
                  [Attr.classes(["InvalidSeq"])],
                  [Node.text(String.sub(s, 0, 2))],
                ),
                ...go(Text(String.sub(s, 2, String.length(s) - 2))),
              ];
            }
          | _ =>
            let ch1 = s.[1];
            if (String.length(s) >= 4) {
              let ch2 = s.[2];
              let ch3 = s.[3];
              if ((ch1 >= '0' && ch1 <= '9')
                  && (ch2 >= '0' && ch2 <= '9')
                  && ch3 >= '0'
                  && ch3 <= '9') {
                if (int_of_string(String.sub(s, 1, 3)) < 256) {
                  [
                    Node.text(String.sub(s, 0, 4)),
                    ...go(Text(String.sub(s, 4, String.length(s) - 4))),
                  ];
                } else {
                  [
                    Node.span(
                      [Attr.classes(["InvalidSeq"])],
                      [Node.text(String.sub(s, 0, 4))],
                    ),
                    ...go(Text(String.sub(s, 4, String.length(s) - 4))),
                  ];
                };
              } else {
                [
                  Node.span(
                    [Attr.classes(["InvalidSeq"])],
                    [Node.text(String.sub(s, 0, 2))],
                  ),
                  ...go(Text(String.sub(s, 2, String.length(s) - 2))),
                ];
              };
            } else {
              [
                Node.span(
                  [Attr.classes(["InvalidSeq"])],
                  [Node.text(String.sub(s, 0, 2))],
                ),
                ...go(Text(String.sub(s, 2, String.length(s) - 2))),
              ];
            };
          }
        | _ => [
            Node.text(String.sub(s, 0, 1)),
            ...go(Text(String.sub(s, 1, String.length(s) - 1))),
          ]
        };
      }
    | Cat(l1, l2) => go(l1) @ go(l2)
    | Linebreak => [Node.br([])]
    | Align(l) => [Node.div([Attr.classes(["Align"])], go(l))]
    | Annot(Collapsed, l) => [
        Node.span([Attr.classes(["Collapsed"])], go(l)),
      ]
    | Annot(HoleLabel, l) => [
        Node.span([Attr.classes(["HoleLabel"])], go(l)),
      ]
    | Annot(Delim, l) => [
        Node.span([Attr.classes(["code-delim"])], go(l)),
      ]
    | Annot(NonEmptyHole(_), l) => [
        Node.span([Attr.classes(["InHole"])], go(l)),
      ]
    | Annot(InconsistentBranches(_), l) => [
        Node.span([Attr.classes(["InconsistentBranches"])], go(l)),
      ]
    | Annot(VarHole(_), l) => [
        Node.span([Attr.classes(["InVarHole"])], go(l)),
      ]
    | Annot(EmptyHole(selected, inst), l) => [
        Node.span(
          [
            Attr.classes(["EmptyHole", ...selected ? ["selected"] : []]),
            Attr.on_click(_ =>
              inject(Update.Action.SelectHoleInstance(inst))
            ),
          ],
          go(l),
        ),
      ]
    | Annot(FailedCastDelim, l) => [
        Node.span([Attr.classes(["FailedCastDelim"])], go(l)),
      ]
    | Annot(FailedCastDecoration, l) => [
        Node.span([Attr.classes(["FailedCastDecoration"])], go(l)),
      ]
    | Annot(CastDecoration, l) => [
        Node.div([Attr.classes(["CastDecoration"])], go(l)),
      ]
    | Annot(InvalidOpDecoration, l) => [
        Node.span([Attr.classes(["InvalidOpDecoration"])], go(l)),
      ]
    };
  Node.div([Attr.classes(["code", "DHCode"])], go(l));
};

let view =
    (
      ~inject,
      ~show_casts: bool,
      ~show_fn_bodies: bool,
      ~show_case_clauses: bool,
      ~selected_instance: option(HoleInstance.t),
      ~width: int,
      ~pos=0,
      d: DHExp.t,
    )
    : Vdom.Node.t => {
  d
  |> DHDoc.Exp.mk(
       ~show_casts,
       ~show_fn_bodies,
       ~show_case_clauses,
       ~enforce_inline=false,
       ~selected_instance,
     )
  |> LayoutOfDoc.layout_of_doc(~width, ~pos)
  |> OptUtil.get(() =>
       failwith("unimplemented: view_of_dhexp on layout failure")
     )
  |> view_of_layout(~inject);
};

let view_of_hole_instance =
    (
      ~inject,
      ~width: int,
      ~pos=0,
      ~selected_instance,
      (u, i): HoleInstance.t,
    )
    : Vdom.Node.t =>
  view(
    ~inject,
    ~show_casts=false,
    ~show_fn_bodies=false,
    ~show_case_clauses=false,
    ~selected_instance,
    ~width,
    ~pos,
    DHExp.EmptyHole(u, i, []),
  );

let view_of_var = x => Vdom.Node.text(x);
