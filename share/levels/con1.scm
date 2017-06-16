;;; steep

(set-track-name (_ "Steep"))
(set-author "Russell Valentine")

(start-time 120) 
(set-start-position 252 252)
(add-goal 229 227 #f "con2")

(add-flag 247 219 100 #t 0.1)
(add-flag 246 219 100 #t 0.1)
(add-flag 245 219 150 #t 0.1)

(add-modpill 253 216 *mod-jump* 20 0)
