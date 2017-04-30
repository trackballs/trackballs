;;; DevNull- level 1

(day)                           ;; Sets daylight for this level. M
(set-track-name "Strange indeed!")         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 120)                ;; We have two minutes to complete levelM
(set-start-position 255.5 255.5) ;; Where the player appearsM
(add-goal 230 220 #t "dn2")     ;; Where we should go (200,200) and whichlevel
                                ;; play when we are finished.M
(fog 0.1)
(diamond 235.5 235.5)

(if (= (difficulty) *hard*)
    (new-mr-black 230 230))
(if (= (difficulty) *easy*)
    (add-modpill 234.5 233.5 *mod-spike* 10 30))

(add-flag 240 240 150 1 0.1)
(add-flag 237 237 250 1 0.1)