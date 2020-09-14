/**
 * A Hazel program ready for user interaction.
 * Contains, in addition to `Statics.edit_state`,
 * user interface state such as the current width of
 * the editor, whether the editor is focused, etc.
 */
[@deriving sexp]
type t =
  pri {
    edit_state: Statics.edit_state,
    width: int,
    start_col_of_vertical_movement: option(int),
    is_focused: bool,
  };

let mk: (~width: int, ~is_focused: bool=?, Statics.edit_state) => t;

let focus: t => t;
let blur: t => t;

let get_zexp: t => ZExp.t;
let get_uhexp: t => UHExp.t;

let get_steps: t => CursorPath.steps;

/**
 * Raised when `CursorInfo_Exp.syn_cursor_info` returns None
 * (indicates a bug, either in that function or in Action
 * because Action needs to return a well-typed edit state)
 */
exception MissingCursorInfo;
let get_cursor_info: t => CursorInfo.t;

/**
 * Raised when edit state does not elaborate
 * (indicates a bug, either in that function or in Action
 * because Action needs to return a well-typed edit state) */
exception DoesNotElaborate;
let get_expansion: t => DHExp.t;

/**
 * Raised when evaluation fails with the InvalidInput output
 * (indicates a bug, either in that function or in Action
 * because Action needs to return a well-typed edit state)
 */
exception InvalidInput;
let get_result: t => Result.t;

/**
 * Raised when an attempted edit action does not succeed
 */
exception FailedAction;
exception CursorEscaped;
let perform_edit_action: (Action.t, t) => t;
let move_via_key:
  (
    ~measure_program_get_doc: bool,
    ~measure_layoutOfDoc_layout_of_doc: bool,
    ~memoize_doc: bool,
    MoveKey.t,
    t
  ) =>
  (t, Action.t);
let move_via_click:
  (
    ~measure_program_get_doc: bool,
    ~measure_layoutOfDoc_layout_of_doc: bool,
    ~memoize_doc: bool,
    (CursorMap.Row.t, CursorMap.Col.t),
    t
  ) =>
  (t, Action.t);

exception HoleNotFound;
let move_to_hole: (MetaVar.t, t) => Action.t;

/**
 * `move_to_case_branch(steps, n)` returns an action that moves the cursor to
 * the `n`th branch in case expression found at `steps` (when the user
 * clicks on a branch type in the error message for a case expression with
 * inconsistent branches)
 */
let move_to_case_branch: (CursorPath.steps, int) => Action.t;

let get_layout:
  (
    ~measure_program_get_doc: bool,
    ~measure_layoutOfDoc_layout_of_doc: bool,
    ~memoize_doc: bool,
    t
  ) =>
  UHLayout.t;
/**
 * Returns a `UHLayout.t` that has been decorated with the caret,
 * current term, and variable uses. (Will be refactored away.)
 */
let get_decorated_layout:
  (
    ~measure_program_get_doc: bool,
    ~measure_layoutOfDoc_layout_of_doc: bool,
    ~memoize_doc: bool,
    t
  ) =>
  UHLayout.t;

let get_cursor_map_z:
  (
    ~measure_program_get_doc: bool,
    ~measure_layoutOfDoc_layout_of_doc: bool,
    ~memoize_doc: bool,
    t
  ) =>
  (CursorMap.t, CursorMap.binding);

let cursor_on_exp_hole: t => option(MetaVar.t);
