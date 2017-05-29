;;; steep

(set-track-name (_ "Steep"))
(set-author "Russell Valentine")

(start-time 120) 
(set-start-position 252.5 252.5)
(add-goal 229 227 #f "con2")

(add-flag 247 219 100 1 0.1)
(add-flag 246 219 100 1 0.1)
(add-flag 245 219 150 1 0.1)

(add-modpill 253.5 216.5 *mod-jump* 20 0)
