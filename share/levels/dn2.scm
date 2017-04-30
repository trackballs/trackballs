;;; DevNull- level 2

(day)                           ;; Sets daylight for this level. M
(set-track-name "Horrible")         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 60)                ;; We have two minutes to complete levelM
(set-start-position 251.5 254.5) ;; Where the player appearsM
(add-goal 251 205 #t "dn3")     ;; Where we should go (200,200) and whichlevel
                                ;; play when we are finished.M
(fog 0.2)

(if (= (difficulty) *hard*)
    (new-mr-black 252 205))

(add-flag 252 253 250 1 0.1)
(add-flag 251 253 250 1 0.1)
(add-flag 251 235 150 1 0.1)
(add-flag 252 235 150 1 0.1)