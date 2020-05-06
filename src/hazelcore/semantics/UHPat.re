open Sexplib.Std;

exception FreeVarInPat;

[@deriving sexp]
type operator = Operators.Pat.t;

[@deriving sexp]
type t = opseq
and opseq = OpSeq.t(operand, operator)
and operand =
  | EmptyHole(MetaVar.t)
  | Wild(ErrStatus.t)
  | Var(ErrStatus.t, VarErrStatus.t, Var.t)
  | IntLit(ErrStatus.t, string)
  | FloatLit(ErrStatus.t, string)
  | BoolLit(ErrStatus.t, bool)
  | ListNil(ErrStatus.t)
  | Parenthesized(t)
  | Inj(ErrStatus.t, InjSide.t, t);

[@deriving sexp]
type skel = OpSeq.skel(operator);
[@deriving sexp]
type seq = OpSeq.seq(operand, operator);

let var =
    (
      ~err: ErrStatus.t=NotInHole,
      ~var_err: VarErrStatus.t=NotInVarHole,
      x: Var.t,
    )
    : operand =>
  Var(err, var_err, x);

let wild = (~err: ErrStatus.t=NotInHole, ()) => Wild(err);

let boollit = (~err: ErrStatus.t=NotInHole, b: bool) => BoolLit(err, b);

let intlit = (~err: ErrStatus.t=NotInHole, n: string) => IntLit(err, n);

let floatlit = (~err: ErrStatus.t=NotInHole, f: string) => FloatLit(err, f);

let listnil = (~err: ErrStatus.t=NotInHole, ()) => ListNil(err);

let rec get_tuple_elements: skel => list(skel) =
  fun
  | BinOp(_, Comma, skel1, skel2) =>
    get_tuple_elements(skel1) @ get_tuple_elements(skel2)
  | skel => [skel];

let rec make_tuple =
        (~err: ErrStatus.t=NotInHole, elements: list(skel)): skel =>
  switch (elements) {
  | [] => failwith("make_tuple: expected at least 1 element")
  | [skel] => skel
  | [skel, ...skels] => BinOp(err, Comma, skel, make_tuple(skels))
  };

let injection = (~err: ErrStatus.t=NotInHole, side: InjSide.t, x: t) =>
  Inj(err, side, x);

/* helper function for constructing a new empty hole */
let new_EmptyHole = (u_gen: MetaVarGen.t): (operand, MetaVarGen.t) => {
  let (u, u_gen) = MetaVarGen.next(u_gen);
  (EmptyHole(u), u_gen);
};

let new_EmptyHoles = ListUtil.iterate(new_EmptyHole);

let is_EmptyHole =
  fun
  | EmptyHole(_) => true
  | _ => false;

let rec get_err_status = (p: t) => get_err_status_opseq(p)
and get_err_status_opseq = opseq =>
  OpSeq.get_err_status(~get_err_status_operand, opseq)
and get_err_status_operand =
  fun
  | EmptyHole(_) => NotInHole
  | Wild(err)
  | Var(err, _, _)
  | IntLit(err, _)
  | FloatLit(err, _)
  | BoolLit(err, _)
  | ListNil(err)
  | Inj(err, _, _) => err
  | Parenthesized(p) => get_err_status(p);

let rec set_err_status = (err: ErrStatus.t, p: t): t =>
  p |> set_err_status_opseq(err)
and set_err_status_opseq = (err, opseq) =>
  OpSeq.set_err_status(~set_err_status_operand, err, opseq)
and set_err_status_operand = (err, operand) =>
  switch (operand) {
  | EmptyHole(_) => operand
  | Wild(_) => Wild(err)
  | Var(_, var_err, x) => Var(err, var_err, x)
  | IntLit(_, n) => IntLit(err, n)
  | FloatLit(_, f) => FloatLit(err, f)
  | BoolLit(_, b) => BoolLit(err, b)
  | ListNil(_) => ListNil(err)
  | Inj(_, inj_side, p) => Inj(err, inj_side, p)
  | Parenthesized(p) => Parenthesized(set_err_status(err, p))
  };

let is_inconsistent = (p: t): bool =>
  switch (get_err_status(p)) {
  | InHole(TypeInconsistent, _) => true
  | _ => false
  };

/* put p in a new hole, if it is not already in a hole */
let rec make_inconsistent = (u_gen: MetaVarGen.t, p: t): (t, MetaVarGen.t) =>
  make_inconsistent_opseq(u_gen, p)
and make_inconsistent_opseq =
    (u_gen: MetaVarGen.t, opseq: opseq): (opseq, MetaVarGen.t) =>
  opseq |> OpSeq.make_inconsistent(~make_inconsistent_operand, u_gen)
and make_inconsistent_operand =
    (u_gen: MetaVarGen.t, operand: operand): (operand, MetaVarGen.t) =>
  switch (operand) {
  // already in hole
  | EmptyHole(_)
  | Wild(InHole(TypeInconsistent, _))
  | Var(InHole(TypeInconsistent, _), _, _)
  | IntLit(InHole(TypeInconsistent, _), _)
  | FloatLit(InHole(TypeInconsistent, _), _)
  | BoolLit(InHole(TypeInconsistent, _), _)
  | ListNil(InHole(TypeInconsistent, _))
  | Inj(InHole(TypeInconsistent, _), _, _) => (operand, u_gen)
  // not in hole
  | Wild(NotInHole | InHole(WrongLength, _))
  | Var(NotInHole | InHole(WrongLength, _), _, _)
  | IntLit(NotInHole | InHole(WrongLength, _), _)
  | FloatLit(NotInHole | InHole(WrongLength, _), _)
  | BoolLit(NotInHole | InHole(WrongLength, _), _)
  | ListNil(NotInHole | InHole(WrongLength, _))
  | Inj(NotInHole | InHole(WrongLength, _), _, _) =>
    let (u, u_gen) = u_gen |> MetaVarGen.next;
    let set_operand =
      operand |> set_err_status_operand(InHole(TypeInconsistent, u));
    (set_operand, u_gen);
  | Parenthesized(p) =>
    let (set_p, u_gen) = p |> make_inconsistent(u_gen);
    (Parenthesized(set_p), u_gen);
  };

let associate = (seq: seq) => {
  let (skel_str, _) = Skel.make_skel_str(seq, Operators.Pat.to_parse_string);
  let lexbuf = Lexing.from_string(skel_str);
  SkelPatParser.skel_pat(SkelPatLexer.read, lexbuf);
};

let mk_OpSeq = OpSeq.mk(~associate);

let patterns_of_type =
    (u_gen: MetaVarGen.t, ty: HTyp.t): (list(t), MetaVarGen.t) =>
  OpSeq.(
    switch (ty) {
    | HTyp.Hole
    | Arrow(_, _) => ([], u_gen)
    | Int
    | Float =>
      let zero =
        if (ty == Int) {
          intlit("0");
        } else {
          floatlit("0.");
        };
      ([zero, wild()] |> List.map(wrap), u_gen);
    | Bool => ([true, false] |> List.map(b => b |> boollit |> wrap), u_gen)
    | Prod([]) => ([], u_gen)
    | Prod(tys) =>
      let (firstHole, u_gen) = u_gen |> new_EmptyHole;
      // List.length(tys) should be >= 1, because we caught the unit-case above and tuple should otherwise have at least 2 elements
      let (holes, u_gen) = new_EmptyHoles(List.length(tys) - 1, u_gen);
      let opseq =
        holes
        |> List.map(hole => (Operators.Pat.Comma, hole))
        |> Seq.mk(firstHole)
        |> mk_OpSeq;
      ([opseq], u_gen);
    | Sum(_, _) =>
      let (holes, u_gen) = new_EmptyHoles(2, u_gen);
      let patterns =
        List.map2(
          (side, hole) => wrap(injection(side, wrap(hole))),
          InjSide.[L, R],
          holes,
        );
      (patterns, u_gen);
    | List(_) =>
      let (hole1, u_gen) = u_gen |> new_EmptyHole;
      let (hole2, u_gen) = u_gen |> new_EmptyHole;
      (
        [wrap(listnil()), wrap_operator(hole1, Operators.Pat.Cons, hole2)],
        u_gen,
      );
    }
  );

let text_operand =
    (u_gen: MetaVarGen.t, shape: TextShape.t): (operand, MetaVarGen.t) =>
  switch (shape) {
  | Underscore => (wild(), u_gen)
  | IntLit(n) => (intlit(n), u_gen)
  | FloatLit(n) => (floatlit(n), u_gen)
  | BoolLit(b) => (boollit(b), u_gen)
  | Var(x) => (var(x), u_gen)
  | ExpandingKeyword(kw) =>
    let (u, u_gen) = u_gen |> MetaVarGen.next;
    (
      var(~var_err=InVarHole(Free, u), kw |> ExpandingKeyword.to_string),
      u_gen,
    );
  };
