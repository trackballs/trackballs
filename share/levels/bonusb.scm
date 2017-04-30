(set-track-name "bonus level")
(set-author "ulrik")
(start-time 60)
(set-start-position 250.5 247.5)

(add-flag 240 243 50 1 0.5)
(add-flag 231 246 50 1 0.5)
(add-flag 225 243 50 1 0.5)
(add-flag 218 246 50 1 0.5)
(add-flag 212 244 50 1 0.5)
(add-flag 206 247 50 1 0.5)
(add-flag 199 242 50 1 0.5)
(add-flag 192 245 50 1 0.5)
(add-flag 187 247 50 1 0.5)
(add-flag 178 245 50 1 0.5)
(add-flag 172 244 50 1 0.5)
(add-flag 165 249 50 1 0.5)

(add-modpill 232.5 249.5 *mod-extra-life* 10 0)
(add-modpill 178.5 248.5 *mod-extra-life* 10 0)
(add-modpill 204.5 244.5 *mod-extra-life* 10 0)

(add-goal 149 245 #t bonus-goto)
(add-goal 149 246 #t bonus-goto)
(add-flag 157 245 5000 0 0.5)
(add-flag 157 246 5000 0 0.5)

(set-primary-color (sign "you made it to the end!" 1 20 -1 149 246) 0.0 1.0 0.0)
 
(set-bonus-level bonus-goto)

(restart-time 0)

(jump 0.0)
