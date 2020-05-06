let is_empty = String.equal("");

let cat = String.concat("");

let replicat = (n: int, s: string) => cat(ListUtil.replicate(n, s));

/**
 * A string of length n has caret positions 0 through n,
 * where 0 places the caret at the start and n places
 * the caret at the end. Split s at caret_index.
 *
 * TODO rename to split
 */
let split_string = (caret_index: int, s: string): (string, string) => (
  String.sub(s, 0, caret_index),
  String.sub(s, caret_index, String.length(s) - caret_index),
);

let insert = (caret_index: int, insert_s: string, s: string): string => {
  let (l, r) = s |> split_string(caret_index);
  l ++ insert_s ++ r;
};

let backspace = (caret_index: int, s: string): string => {
  let l = String.sub(s, 0, caret_index - 1);
  let r = String.sub(s, caret_index, String.length(s) - caret_index);
  l ++ r;
};

let delete = (caret_index: int, s: string): string => {
  let l = String.sub(s, 0, caret_index);
  let r = String.sub(s, caret_index + 1, String.length(s) - caret_index - 1);
  l ++ r;
};

let utf8_length = CamomileLibrary.UTF8.length;

let num_leading_zeros = (num: string): int => {
  let rec helper = (index: int) =>
    if (index == String.length(num) || num.[index] != '0') {
      0;
    } else {
      1 + helper(index + 1);
    };
  helper(0);
};

let num_trailing_zeros = (num: string): int => {
  let rec helper = (index: int) =>
    if (index < 0 || num.[index] != '0') {
      0;
    } else {
      1 + helper(index - 1);
    };
  helper(String.length(num) - 1);
};

let strip_underscores = (s: string): string => {
  let rec helper = (index: int, stripped: string): string =>
    if (index == String.length(s)) {
      stripped;
    } else {
      switch (String.make(1, s.[index])) {
      | "_" => helper(index + 1, stripped)
      | c => helper(index + 1, stripped ++ c)
      };
    };
  helper(0, "");
};
