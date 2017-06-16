;;; DevNull- level 1

(day)                           ;; Sets daylight for this level. M
(set-track-name (_ "Strange indeed!"))         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 120)                ;; We have two minutes to complete levelM
(set-start-position 254 254) ;; Where the player appearsM
(add-goal 229 229 #t "dn2")     ;; Where we should go (200,200) and whichlevel
                                ;; play when we are finished.M
(fog 0.1)
(diamond 234 234)

(if (= (difficulty) *hard*)
    (new-mr-black 228.5 228.5))
(if (= (difficulty) *easy*)
    (add-modpill 233 232 *mod-spike* 10 30))

(add-flag 239 239 150 #t 0.1)
(add-flag 236 236 250 #t 0.1)
