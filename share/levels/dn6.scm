;;; DevNull- levelu 6 -final
;; Tnx to Mathias Broxvall for this game! it rox! :-)
;; And tnx also to Dietrich Radel for his cool ogg's and for his Castle level!
;;; Enjoy your life! ;-)      

(day)                           ;; Sets daylight for this level. M
(set-track-name "Final and the last")         ;; The name of the levelM
(set-author "Francek") ;;
(start-time 210)                ;; We have two minutes to complete levelM
(set-start-position 200.5 200.5) ;; Where the player appearsM

(diamond 201.5 179.5)

(trigger 200.5 200.5 0.6 (lambda()(camera-angle 0.0 0.3)))
(trigger 198.5 199.5 0.4 (lambda()(camera-angle 0.3 0.1)))
(trigger 200.5 202.5 0.4 (lambda()(camera-angle 1.0 0.15)))
(trigger 202.5 199.5 0.5 (lambda()(camera-angle 0.0 0.0)))
(trigger 201.5 287.5 0.5 (lambda()(camera-angle 0.3 0.0)))

(thick-fog)
(fog-color 0.1 0.1 0.1)

(add-teleport 196 173 200 200 0.3)

(define b1 (new-mr-black 199 180))
(set-horizon b1 4.0)
(set-acceleration b1 6.0)
(define b2 (new-mr-black 199 177))
(set-horizon b2 4.0)
(set-acceleration b2 8.0) ;; :P
(define b3 (new-mr-black 202 180))
(set-horizon b3 4.0)
(set-acceleration b3 7.0)
(define b4 (new-mr-black 209 177))
(set-horizon b4 2.5)

(sign "Switch twice" 1.0 0.0 0.0 218.5 156.5)

(if (= (difficulty) *hard*)
    (new-mr-black 200 201))
(if (= (difficulty) *hard*)
    (new-mr-black 201 201))
(if (= (difficulty) *hard*)
    (new-mr-black 199 199))
(if (= (difficulty) *hard*)
    (new-mr-black 201 180))
(if (= (difficulty) *normal*)
    (new-mr-black 201 180))
(if (= (difficulty) *normal*)
    (new-mr-black 199 199))


(define spike2 (add-spike 219.9 157.9 3.0 -0.00))
(define spike3 (add-spike 217.1 155.1 3.0 -0.00))
(define spike4 (add-spike 198.1 176.1 3.0 -0.00))

(switch 223.5 151.5
        (lambda () (set-speed spike2 0.2))
		(lambda () (set-speed spike2 3.0)))
(switch 223.5 161.5
        (lambda () (set-speed spike3 0.2))
        (lambda () (set-speed spike3 3.0)))
(switch 213.5 151.5
        (lambda () (set-speed spike4 0.2))
        (lambda () (set-speed spike4 3.0)))

;; the invisible goal, tnx to Dietrich Radel's Elite castle level :-)
(define invisible 'true)
(switch 194.5 172.5
        (lambda () (if invisible (begin (set! invisible #f) (add-goal 201 201 #t ""))))
        (lambda () (new-mr-black 201.5 200.5)))

(sign "Switch twice" 1.0 0.0 0.0 194.5 172.5)


(add-flag 208 177 200 1 0.1)
(add-flag 209 177 300 1 0.1)
(add-flag 210 177 200 1 0.1)
(add-flag 211 177 300 1 0.1)
(add-flag 212 177 200 1 0.1)

(add-flag 213 176 300 1 0.1)

(add-flag 213 165 200 1 0.1)
(add-flag 213 164 300 1 0.1)
(add-flag 213 163 200 1 0.1)
(add-flag 213 162 300 1 0.1)
(add-flag 213 161 200 1 0.1)