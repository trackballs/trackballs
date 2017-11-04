;;; DevNull- levelu 6 -final
;; Tnx to Mathias Broxvall for this game! it rox! :-)
;; And tnx also to Dietrich Radel for his cool ogg's and for his Castle level!
;;; Enjoy your life! ;-)      

(day)                           ;; Sets daylight for this level. M
(set-track-name (_ "Final and the last"))         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 210)                ;; We have two minutes to complete levelM
(set-start-position 200 200) ;; Where the player appearsM

(diamond 201 179)

(trigger 200 200 0.6 (lambda()(camera-angle 0.0 0.3)))
(trigger 198 199 0.4 (lambda()(camera-angle 0.3 0.1)))
(trigger 200 202 0.4 (lambda()(camera-angle 1.0 0.15)))
(trigger 202 199 0.5 (lambda()(camera-angle 0.0 0.0)))
(trigger 201 287 0.5 (lambda()(camera-angle 0.3 0.0)))

(thick-fog)
(fog-color 0.1 0.1 0.1)

(add-teleport 196 173 200 200 0.3)

(define b1 (new-mr-black 198.5 179.5))
(set-horizon b1 4.0)
(set-acceleration b1 6.0)
(define b2 (new-mr-black 198.5 176.5))
(set-horizon b2 4.0)
(set-acceleration b2 8.0) ;; :P
(define b3 (new-mr-black 201.5 179.5))
(set-horizon b3 4.0)
(set-acceleration b3 7.0)
(define b4 (new-mr-black 208.5 176.5))
(set-horizon b4 2.5)

(sign (_ "Switch twice") 1.0 0.0 0.0 218 156)

(if (= (difficulty) *hard*)
    (new-mr-black 199.5 200.5))
(if (= (difficulty) *hard*)
    (new-mr-black 200.5 200.5))
(if (= (difficulty) *hard*)
    (new-mr-black 198.5 198.5))
(if (= (difficulty) *hard*)
    (new-mr-black 200.5 170.5))
(if (= (difficulty) *normal*)
    (new-mr-black 200.5 170.5))
(if (= (difficulty) *normal*)
    (new-mr-black 198.5 198.5))


(define spike2 (add-spike 219.4 157.4 3.0 -0.00))
(define spike3 (add-spike 216.6 154.6 3.0 -0.00))
(define spike4 (add-spike 197.6 175.6 3.0 -0.00))

(switch 223 151
        (lambda () (set-speed spike2 0.2))
        (lambda () (set-speed spike2 3.0)))
(switch 223 161
        (lambda () (set-speed spike3 0.2))
        (lambda () (set-speed spike3 3.0)))
(switch 213 151
        (lambda () (set-speed spike4 0.2))
        (lambda () (set-speed spike4 3.0)))

;; the invisible goal, tnx to Dietrich Radel's Elite castle level :-)
(define invisible 'true)
(switch 194 172
        (lambda () (if invisible (begin (set! invisible #f) (add-goal 201 201 #t ""))))
        (lambda () (new-mr-black 201 200)))

(sign (_ "Switch twice") 1.0 0.0 0.0 194 172)


(add-flag 208 177 200 #t 0.1)
(add-flag 209 177 300 #t 0.1)
(add-flag 210 177 200 #t 0.1)
(add-flag 211 177 300 #t 0.1)
(add-flag 212 177 200 #t 0.1)

(add-flag 213 176 300 #t 0.1)

(add-flag 213 165 200 #t 0.1)
(add-flag 213 164 300 #t 0.1)
(add-flag 213 163 200 #t 0.1)
(add-flag 213 162 300 #t 0.1)
(add-flag 213 161 200 #t 0.1)
