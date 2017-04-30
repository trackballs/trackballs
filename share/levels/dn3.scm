;;; DevNull- level 3

(day)                           ;; Sets daylight for this level. M
(set-track-name "Water jumping")         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 100)                ;; We have two minutes to complete levelM
(set-start-position 253.5 254.5) ;; Where the player appearsM
(add-goal 231 184 #t "dn4")     ;; Where we should go (200,200) and which level"l
                                ;; play when we are finished.M
(fog 0.3)
(diamond 253.5 184.5)

(if (= (difficulty) *hard*)
    (new-mr-black 253 230))
(if (= (difficulty) *normal*)
    (add-modpill 253.5 251.5 *mod-spike* 15 30))

(add-flag 237 184 200 1 0.1)
(add-flag 243 184 150 1 0.1)
(add-flag 240 184 150 1 0.1)
(add-flag 253 240 250 1 0.1)