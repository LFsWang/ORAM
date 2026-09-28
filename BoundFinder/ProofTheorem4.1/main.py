from math import *

# Lemma 4.10
def A(t, Z):
    return exp(-t*Z) / (2 - exp(t))
def B(t, Z):
    return exp(exp(t)-1-t*Z)

t = 0.530
assert(t < log(2))
print(f"select t = {t:.3f}")

# eq 4.13 
ent = exp(-t) # e^(-t)
print(f"(4.13)  e^(-tR) * g_0 = {ent:.5f}^R * g_0")

# eq 4.14

w_L = B(t, 4)
w_Ln1 = w_Ln2 = w_Ln3 = A(t, 4)

print( "(4.14)")
print(f"        w_L = B(t, 4) = {w_L:.4f}") 
print(f"        w_Ln1 = w_Ln2 = w_Ln3 = A(t, 4) = {w_Ln1:.4f}")

w_j = A(t, 5)
print(f"        w_j = A(t, 5) = {w_j:.4f}")

print( "(4.15)")
g_L = w_L
print(f"        g_L = w_L = {g_L:.4f}") 
g_Ln1 = w_Ln1 * ((1+g_L)**2)
print(f"        g_Ln1 = w_Ln1 * (1+g_L)**2 = {g_Ln1:.4f}") 
g_Ln2 = w_Ln2 * ((1+g_Ln1)**2)
print(f"        g_Ln2 = w_Ln2 * (1+g_Ln1)**2 = {g_Ln2:.4f}") 
g_Ln3 = w_Ln3 * ((1+g_Ln2)**2)
print(f"        g_Ln3 = w_Ln3 * (1+g_Ln2)**2 = {g_Ln3:.4f}") 


w = A(t, 5)
g_p = ((1 - 2 * w) + sqrt(1 - 4 * w)) / (2 * w)
print(f"        g_p = {g_p:.5f}")
print(f"              g_Ln3 < g_p  : {g_Ln3 < g_p}")
