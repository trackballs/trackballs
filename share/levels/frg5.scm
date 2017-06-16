(set-track-name (_ "Upside Down"))
(set-author "Ulrik Enstad")

(set-start-position 172 172)

(new-baby 184 185)
(new-baby 186 183)
(new-baby 182 187)
(new-baby 182 184)

(define pipe1 (pipe 188 188 2.1 188.5 188.5 2.1 0.4))
(define pipe2 (pipe-connector 188.5 188.5 2.1 0.4))
(define pipe3 (pipe 188.5 188.5 2.1 189 189 2.7 0.4))
(define pipe4 (pipe-connector 189 189 2.7 0.4))
(define pipe5 (pipe 189 189 2.7 189 188.9 3.6 0.4))
(define pipe6 (pipe-connector 189 188.9 3.6 0.4))
(define pipe7 (pipe 189 188.9 3.6 189 188.1 4.2 0.4))

(set-wind pipe1 16.0 0.0)
(set-wind pipe2 16.0 0.0)
(set-wind pipe3 16.0 0.0)
(set-wind pipe4 16.0 0.0)
(set-wind pipe5 16.0 0.0)
(set-wind pipe6 16.0 0.0)
(set-wind pipe7 16.0 0.0)

(diamond 190.0 182.0)

(define speed (cond
 ((= (difficulty) *easy*) .4)
 ((= (difficulty) *normal*) .3)
 ((= (difficulty) *hard*) .2)
))

(add-cyclic-platform 204 191 204 191 4.9 6.4 1. speed)
(add-cyclic-platform 204 190 204 190 4.9 6.4 6. speed)
(add-cyclic-platform 205 191 205 191 4.9 6.4 3. speed)
(add-cyclic-platform 205 190 205 190 4.9 6.4 5. speed)
(add-cyclic-platform 206 191 206 191 4.9 6.4 2. speed)
(add-cyclic-platform 206 190 206 190 4.9 6.4 4. speed)

(add-cyclic-platform 207 194 207 194 4.9 6.4 1. speed)
(add-cyclic-platform 208 194 208 194 4.9 6.4 4. speed)
(add-cyclic-platform 207 195 207 195 4.9 6.4 2. speed)
(add-cyclic-platform 208 195 208 195 4.9 6.4 3. speed)
(add-cyclic-platform 207 196 207 196 4.9 6.4 6. speed)
(add-cyclic-platform 208 196 208 196 4.9 6.4 5. speed)

(set-wind (pipe 207.5 198.5 5.4 207.5 200.5 6.1 0.4) 8.0 0.0)
(set-wind (pipe-connector 207.5 200.5 6.1 0.4) 8.0 0.0)
(set-wind (pipe 207.5 200.5 6.1 207.6 200.5 7.7 0.4) 8.0 0.0)

(add-goal 211 201 #t "frg6")
(add-goal 211 200 #t "frg6")

(jump 0.0)

(add-flag 208 195 30 #f 1.5)
(add-flag 206 190 30 #f 1.5)
(add-flag 200 188 30 #f 0.5)
(add-flag 189 188 30 #f 0.5)

(cond
 ((= (difficulty) *easy*)
         (start-time 120)
 )
 ((= (difficulty) *normal*)
         (start-time 100)
 )
 ((= (difficulty) *hard*)
         (start-time 60)
 )
)

(restart-time 0)



