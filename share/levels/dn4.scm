;;; DevNull- level 4

(day)                           ;; Sets daylight for this level. M
(set-track-name "Oh my god!")         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 200)                ;; We have two minutes to complete levelM
(set-start-position 250.5 251.5)
(add-goal 241 230 #f "dn5")     ;; Where we should go (200,200) and which level"l
                                ;; play when we are finished.M
(fog 0.4)
(diamond 244.5 236.5)
(add-teleport 241 231 241 213 0.3) ;teleport1
(add-teleport 241 228 221 210 0.3) ;teleport2
(add-teleport 223 213 241 229 0.3) ;teleport4

(sign "Thanks for playing!" 1.1 0.0 0.0 221.5 210.5)

(add-spike 241.5 214.5 0.7 -0.00)
(add-spike 241.5 219.5 0.7 -1.20)
(add-spike 241.5 223.5 0.7 -2.20)

(if (= (difficulty) *hard*)
    (new-mr-black 219.5 212.5))
(if (= (difficulty) *hard*)
	(new-mr-black 223.5 208.5))
(if (= (difficulty) *hard*)
	(new-mr-black 223.5 210.5))
(if (= (difficulty) *hard*)
	(new-mr-black 219.5 208.5))
(if (= (difficulty) *normal*)
    (new-mr-black 219.5 212.5))
(if (= (difficulty) *normal*)
	(new-mr-black 223.5 208.5))

(add-flag 241 215 150 1 0.1)
(add-flag 241 220 150 1 0.1)
(add-flag 241 224 150 1 0.1)

(add-flag 222 212 150 1 0.1)
(add-flag 221 212 150 1 0.1)
(add-flag 220 212 150 1 0.1)
(add-flag 223 212 150 1 0.1)

(add-flag 222 209 150 1 0.1)
(add-flag 222 210 150 1 0.1)
(add-flag 222 211 150 1 0.1)

(add-flag 221 209 100 1 0.1)
(add-flag 220 209 100 1 0.1)
(add-flag 223 208 100 1 0.1)
(add-flag 223 209 100 1 0.1)
(add-flag 220 208 100 1 0.1)
(add-flag 220 211 100 1 0.1)
(add-flag 223 210 100 1 0.1)
(add-flag 223 211 100 1 0.1)

(add-flag 221 211 100 1 0.1)
(add-flag 220 210 100 1 0.1)
(add-flag 223 211 100 1 0.1)

(add-flag 222 208 100 1 0.1)
(add-flag 221 208 100 1 0.1)
(add-flag 219 208 100 1 0.1)
(add-flag 219 209 100 1 0.1)
(add-flag 219 210 100 1 0.1)
(add-flag 219 211 100 1 0.1)
(add-flag 219 212 100 1 0.1)