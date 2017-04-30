;;; steep

(set-track-name "Steep")
(set-author "Russell Valentine")

(start-time 120) 
(set-start-position 253.5 253.5)
(add-goal 230 228 #f "con2")

;;(set-next-level "")

(add-flag 248 220 100 1 0.1)
(add-flag 247 220 100 1 0.1)
(add-flag 246 220 150 1 0.1)

(add-modpill 254.5 217.5 *mod-jump* 20 0)
