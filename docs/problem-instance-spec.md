
# Instance Description

Angelehnt an [QAPLib](http://anjos.mgi.polymtl.ca/qaplib/)

**Separator zwischen Values:** Leerzeichen

**Separator zwischen Listen/Matrizen:** Leerzeile

## Liste von Fabriken F_i

| id | Ausfallwahrscheinlichkeit p_i (float) | Kapazität c_i (int) | x-Koordinate x_i (int) | y-Koordinate y_i (int) |
| --- | ------------------------------------- |---------- | ---------- | ---------- |
| 0 |  0.2 | 10 | 1 | 1 |
|  ... | ...  | ...  | ...  | ...  |
| k |  p_k | c_k | x_k  | x_k  |

## Liste von Maschinen M_i

| id | Größe s_i (int) | redundancy r_i (int, default=1) |
|---|---|---|
| 0 |  3 | 1 |
|  ... | ...  | ...  |
| n |  s_i | r_n | x_n | x_n  |

## Flow Matrix (int, n x n)

|   | 0  | 1 | ... | n |
|----|---|---|---|---|
| 0  | f_00 | f_01 | ... | f_0n
| 1  | f_10 | f_11 | ... | f_1n
| ...| ... | ... | ...  | ...
| n  | f_n0 | f_n1 | ... | f_nn

## Change Over Costs Matrix (int, k x k)

|   | 0  | 1 | ... | k |
|----|---|---|---|---|
| 0  | c_00 | c_01 | ... | c_0k
| 1  | c_10 | c_11 | ... | c_1k
| ...| ... | ... | ...  | ...
| k  | c_k0 | c_k1 | ... | c_kk

# Mini Example

2 Fabriken, 3 Maschinen

```
0 0.1 10 1 1
1 0.2 20 2 2

0 3 1
1 4 1
2 5 1

0 2 2
3 0 4
1 1 0

0 1
5 0
```