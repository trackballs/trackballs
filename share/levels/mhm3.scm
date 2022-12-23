;;; The Castle

(set-track-name (_ "The Castle"))
(set-author "Warren D. Ober")
(set-start-position 101.5 167.5) 
(start-time 600)
(restart-time 30)
;;Weather conditions.

(day)


;Warning sign.

(set-primary-color (sign (_ "Stay on the path or else!") 2.0 -0.1 4.0 113.0 172.0 -3.5) 0.9 0.6 0.0 1.0)



;Pipe over drawbridge

(set-primary-color (pipe 144.0 171.5 -0.5 144.0 166.5 -0.5 1.5) 0.9 0.6 0.0 1.0)


;palace guards.

(define mr1 (new-mr-black 153 172))
(set-acceleration mr1 3.0)
(set-primary-color mr1 1. 1. 1. 1.0)


(define mr2 (new-mr-black 151 165))
(set-acceleration mr2 4.0)
(set-primary-color mr2 1. 1. 1. 1.0)


(define mr3 (new-mr-black 177 140))
(set-acceleration mr3 7.0)
(set-primary-color mr3 1. 1. 1. 1.0)


(define mr4 (new-mr-black 183 136))
(set-acceleration mr4 4.0)
(set-primary-color mr4 1. 1. 1. 1.0)


(define mr5 (new-mr-black 162 195))
(set-acceleration mr5 4.0)
(set-primary-color mr5 1. 1. 1. 1.0)


(define mr6 (new-mr-black 175 195))
(set-acceleration mr6 4.0)
(set-primary-color mr6 1. 1. 1. 1.0)


(define mr7 (new-mr-black 177 165))
(set-acceleration mr7 4.0)
(set-primary-color mr7 1. 1. 1. 1.0)


(define mr8 (new-mr-black 148 59))
(set-acceleration mr8 5.0)
(set-primary-color mr8 1. 1. 1. 1.0)


(define mr9 (new-mr-black 144 136))
(set-acceleration mr9 7.0)
(set-primary-color mr9 1. 1. 1. 1.0)


(define mr10 (new-mr-black 144 200))
(set-acceleration mr10 7.0)
(set-primary-color mr10 1. 1. 1. 1.0)






;hidden power-up


(add-modpill 183 171 *mod-jump* 50 50)





;flags on Aromory and Keep.

(add-flag 151 179 100 #t 0.1)
(add-flag 153 179 100 #t 0.1)
(add-flag 152 178 100 #t 0.1)
(add-flag 151 177 100 #t 0.1)
(add-flag 153 177 100 #t 0.1)
(add-flag 152 176 100 #t 0.1)
(add-flag 153 175 100 #t 0.1)
(add-flag 151 175 100 #t 0.1)


(add-flag 148 161 100 #t 0.1)
(add-flag 150 161 100 #t 0.1)
(add-flag 149 160 100 #t 0.1)
(add-flag 150 159 100 #t 0.1)
(add-flag 148 159 100 #t 0.1)
(add-flag 149 158 100 #t 0.1)
(add-flag 150 157 100 #t 0.1)
(add-flag 148 157 100 #t 0.1)



;savepoints.


(diamond 155.5 140.5)
(diamond 158.5 184.5)



;fountains in garden.


(fountain 167 170.5 -2.0 0.03 0.0 200.0)
(fountain 167 169.5 -2.0 0.03 0.0 200.0)


;; wave in garden.


;(add-heightmodifier corner x y min max freq phase [not1] [not2] [not3]) 


(add-heightmodifier 10 172 179 -1.2 1.0 0.2 -0.4)
(add-heightmodifier 10 173 179 -1.2 1.0 0.2 -0.4)
(add-heightmodifier 10 174 179 -1.2 1.0 0.2 -0.4)

(add-heightmodifier 10 172 178 -1.4 0.8 0.2 0.0)
(add-heightmodifier 10 173 178 -1.4 0.8 0.2 0.0)
(add-heightmodifier 10 174 178 -1.4 0.8 0.2 0.0)

(add-heightmodifier 10 172 177 -1.6 0.6 0.2 0.4)
(add-heightmodifier 10 173 177 -1.6 0.6 0.2 0.4)
(add-heightmodifier 10 174 177 -1.6 0.6 0.2 0.4)

(add-heightmodifier 10 172 176 -1.8 0.4 0.2 0.8)
(add-heightmodifier 10 173 176 -1.8 0.4 0.2 0.8)
(add-heightmodifier 10 174 176 -1.8 0.4 0.2 0.8)

(add-heightmodifier 10 172 175 -2.0 0.2 0.2 1.2)
(add-heightmodifier 10 173 175 -2.0 0.2 0.2 1.2)
(add-heightmodifier 10 174 175 -2.0 0.2 0.2 1.2)

(add-heightmodifier 10 172 174 -2.2 0.0 0.2 1.6)
(add-heightmodifier 10 173 174 -2.2 0.0 0.2 1.6)
(add-heightmodifier 10 174 174 -2.2 0.0 0.2 1.6)

(add-heightmodifier 10 172 173 -2.4 -0.2 0.2 2.0)
(add-heightmodifier 10 173 173 -2.4 -0.2 0.2 2.0)
(add-heightmodifier 10 174 173 -2.4 -0.2 0.2 2.0)

(add-heightmodifier 10 172 172 -2.6 -0.4 0.2 2.4)
(add-heightmodifier 10 173 172 -2.6 -0.4 0.2 2.4)
(add-heightmodifier 10 174 172 -2.6 -0.4 0.2 2.4)

(add-heightmodifier 10 172 171 -2.8 -0.6 0.2 2.8)
(add-heightmodifier 10 173 171 -2.8 -0.6 0.2 2.8)
(add-heightmodifier 10 174 171 -2.8 -0.6 0.2 2.8)

(add-heightmodifier 10 172 170 -3.0 -0.8 0.2 3.2)
(add-heightmodifier 10 173 170 -3.0 -0.8 0.2 3.2)
(add-heightmodifier 10 174 170 -3.0 -0.8 0.2 3.2)

(add-heightmodifier 10 172 169 -3.2 -1.0 0.2 3.6)
(add-heightmodifier 10 173 169 -3.2 -1.0 0.2 3.6)
(add-heightmodifier 10 174 169 -3.2 -1.0 0.2 3.6)

(add-heightmodifier 10 172 168 -3.4 -1.2 0.2 4.0)
(add-heightmodifier 10 173 168 -3.4 -1.2 0.2 4.0)
(add-heightmodifier 10 174 168 -3.4 -1.2 0.2 4.0)


(add-heightmodifier 10 172 167 -3.4 -1.2 0.2 4.4)
(add-heightmodifier 10 173 167 -3.4 -1.2 0.2 4.4)
(add-heightmodifier 10 174 167 -3.4 -1.2 0.2 4.4)

(add-heightmodifier 10 172 166 -3.2 -1.0 0.2 4.8)
(add-heightmodifier 10 173 166 -3.2 -1.0 0.2 4.8)
(add-heightmodifier 10 174 166 -3.2 -1.0 0.2 4.8)

(add-heightmodifier 10 172 165 -3.0 -0.8 0.2 5.2)
(add-heightmodifier 10 173 165 -3.0 -0.8 0.2 5.2)
(add-heightmodifier 10 174 165 -3.0 -0.8 0.2 5.2)

(add-heightmodifier 10 172 164 -2.8 -0.6 0.2 5.6)
(add-heightmodifier 10 173 164 -2.8 -0.6 0.2 5.6)
(add-heightmodifier 10 174 164 -2.8 -0.6 0.2 5.6)

(add-heightmodifier 10 172 163 -2.6 -0.4 0.2 6.0)
(add-heightmodifier 10 173 163 -2.6 -0.4 0.2 6.0)
(add-heightmodifier 10 174 163 -2.6 -0.4 0.2 6.0)

(add-heightmodifier 10 172 162 -2.4 -0.2 0.2 6.4)
(add-heightmodifier 10 173 162 -2.4 -0.2 0.2 6.4)
(add-heightmodifier 10 174 162 -2.4 -0.2 0.2 6.4)

(add-heightmodifier 10 172 161 -2.2 0.0 0.2 6.8)
(add-heightmodifier 10 173 161 -2.2 0.0 0.2 6.8)
(add-heightmodifier 10 174 161 -2.2 0.0 0.2 6.8)

(add-heightmodifier 10 172 160 -2.0 0.2 0.2 7.2)
(add-heightmodifier 10 173 160 -2.0 0.2 0.2 7.2)
(add-heightmodifier 10 174 160 -2.0 0.2 0.2 7.2)

(add-heightmodifier 10 172 159 -1.8 0.4 0.2 7.6)
(add-heightmodifier 10 173 159 -1.8 0.4 0.2 7.6)
(add-heightmodifier 10 174 159 -1.8 0.4 0.2 7.6)

(add-heightmodifier 10 172 158 -1.6 0.6 0.2 8.0)
(add-heightmodifier 10 173 158 -1.6 0.6 0.2 8.0)
(add-heightmodifier 10 174 158 -1.6 0.6 0.2 8.0)

(add-heightmodifier 10 172 157 -1.4 0.8 0.2 8.4)
(add-heightmodifier 10 173 157 -1.4 0.8 0.2 8.4)
(add-heightmodifier 10 174 157 -1.4 0.8 0.2 8.4)

(add-heightmodifier 10 172 156 -1.2 1.0 0.2 8.8)
(add-heightmodifier 10 173 156 -1.2 1.0 0.2 8.8)
(add-heightmodifier 10 174 156 -1.2 1.0 0.2 8.8)





;blinker & ramp.

(define blinker (add-colormodifier 3 159 148 0.5 1.0 0.5 1.0))
(trigger-once 158.5 147.5 0.5 (lambda ()
  (set-cell-heights 164 184 164 186 -0.5 -0.5 0.0 0.0)
  (set-cell-heights 165 184 165 186 0.0 0.0 0.5 0.5)
  (set-cell-heights 166 184 166 186 0.5 0.5 1.0 1.0)
  (set-primary-color (sign (_ "You may cross the Garden.") 0.8 1.0 -0.1 159 148) 0.9 0.6 0.0 1.0)
  (set-onoff blinker #f)
))


;;pipe out of garden.

(define pipe1(pipe 168 182.5 1.5 168 181.5 1.5 0.6))
(set-primary-color pipe1 0.9 0.6 0.0 1.0)

(set-primary-color (pipe-connector 168 181.5 1.5 0.6) 0.9 0.6 0.0 1.0)

(define pipe2(pipe 168 181.5 1.2 168.0 180.5 -5.0 0.5))
(set-primary-color pipe2 0.9 0.6 0.0 1.0)
(set-wind pipe2 -30.0 -9.0)

(set-primary-color (pipe-connector 168.0 180.5 -4.9 0.5) 0.9 0.6 0.0 1.0)

(define pipe3(pipe 168.0 180.5 -4.9 168 173.5 -4.9 0.5))
(set-primary-color pipe3 0.9 0.6 0.0 1.0)


;;End of level

(add-goal 183 141 #f "")


