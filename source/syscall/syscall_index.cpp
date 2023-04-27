/**
 * @file syscall_index.cpp
 * @author Shane Menzies
 * @brief
 * @date 1/1/23
 *
 *
 */

#include "syscall_index.h"

// region syscalls

// Empty syscall for unimplemented syscalls
uint64_t unimplemented(SYSCALL_ARGS) {
    (void)arg_0;
    (void)arg_1;
    (void)arg_2;
    (void)arg_3;
    (void)arg_4;
    (void)arg_5;
    return 0;
}

// endregion

// Syscalls use same numbering as linux's x86_64 syscalls
std_k::function<uint64_t(SYSCALL_ARG_TYPES)> syscall_index[] = {

    // 0 - read
    unimplemented,

    // 1 - write
    unimplemented,

    // 2 - open
    unimplemented,

    // 3 - close
    unimplemented,

    // 4 - temporary filler
    unimplemented,

    // 5 - temporary filler
    unimplemented,

    // 6 - temporary filler
    unimplemented,

    // 7 - temporary filler
    unimplemented,

    // 8 - temporary filler
    unimplemented,

    // 9 - temporary filler
    unimplemented,

    // 10 - temporary filler
    unimplemented,

    // 11 - temporary filler
    unimplemented,

    // 12 - temporary filler
    unimplemented,

    // 13 - temporary filler
    unimplemented,

    // 14 - temporary filler
    unimplemented,

    // 15 - temporary filler
    unimplemented,

    // 16 - temporary filler
    unimplemented,

    // 17 - temporary filler
    unimplemented,

    // 18 - temporary filler
    unimplemented,

    // 19 - temporary filler
    unimplemented,

    // 20 - temporary filler
    unimplemented,

    // 21 - temporary filler
    unimplemented,

    // 22 - temporary filler
    unimplemented,

    // 23 - temporary filler
    unimplemented,

    // 24 - temporary filler
    unimplemented,

    // 25 - temporary filler
    unimplemented,

    // 26 - temporary filler
    unimplemented,

    // 27 - temporary filler
    unimplemented,

    // 28 - temporary filler
    unimplemented,

    // 29 - temporary filler
    unimplemented,

    // 30 - temporary filler
    unimplemented,

    // 31 - temporary filler
    unimplemented,

    // 32 - temporary filler
    unimplemented,

    // 33 - temporary filler
    unimplemented,

    // 34 - temporary filler
    unimplemented,

    // 35 - temporary filler
    unimplemented,

    // 36 - temporary filler
    unimplemented,

    // 37 - temporary filler
    unimplemented,

    // 38 - temporary filler
    unimplemented,

    // 39 - temporary filler
    unimplemented,

    // 40 - temporary filler
    unimplemented,

    // 41 - temporary filler
    unimplemented,

    // 42 - temporary filler
    unimplemented,

    // 43 - temporary filler
    unimplemented,

    // 44 - temporary filler
    unimplemented,

    // 45 - temporary filler
    unimplemented,

    // 46 - temporary filler
    unimplemented,

    // 47 - temporary filler
    unimplemented,

    // 48 - temporary filler
    unimplemented,

    // 49 - temporary filler
    unimplemented,

    // 50 - temporary filler
    unimplemented,

    // 51 - temporary filler
    unimplemented,

    // 52 - temporary filler
    unimplemented,

    // 53 - temporary filler
    unimplemented,

    // 54 - temporary filler
    unimplemented,

    // 55 - temporary filler
    unimplemented,

    // 56 - temporary filler
    unimplemented,

    // 57 - temporary filler
    unimplemented,

    // 58 - temporary filler
    unimplemented,

    // 59 - temporary filler
    unimplemented,

    // 60 - temporary filler
    unimplemented,

    // 61 - temporary filler
    unimplemented,

    // 62 - temporary filler
    unimplemented,

    // 63 - temporary filler
    unimplemented,

    // 64 - temporary filler
    unimplemented,

    // 65 - temporary filler
    unimplemented,

    // 66 - temporary filler
    unimplemented,

    // 67 - temporary filler
    unimplemented,

    // 68 - temporary filler
    unimplemented,

    // 69 - temporary filler
    unimplemented,

    // 70 - temporary filler
    unimplemented,

    // 71 - temporary filler
    unimplemented,

    // 72 - temporary filler
    unimplemented,

    // 73 - temporary filler
    unimplemented,

    // 74 - temporary filler
    unimplemented,

    // 75 - temporary filler
    unimplemented,

    // 76 - temporary filler
    unimplemented,

    // 77 - temporary filler
    unimplemented,

    // 78 - temporary filler
    unimplemented,

    // 79 - temporary filler
    unimplemented,

    // 80 - temporary filler
    unimplemented,

    // 81 - temporary filler
    unimplemented,

    // 82 - temporary filler
    unimplemented,

    // 83 - temporary filler
    unimplemented,

    // 84 - temporary filler
    unimplemented,

    // 85 - temporary filler
    unimplemented,

    // 86 - temporary filler
    unimplemented,

    // 87 - temporary filler
    unimplemented,

    // 88 - temporary filler
    unimplemented,

    // 89 - temporary filler
    unimplemented,

    // 90 - temporary filler
    unimplemented,

    // 91 - temporary filler
    unimplemented,

    // 92 - temporary filler
    unimplemented,

    // 93 - temporary filler
    unimplemented,

    // 94 - temporary filler
    unimplemented,

    // 95 - temporary filler
    unimplemented,

    // 96 - temporary filler
    unimplemented,

    // 97 - temporary filler
    unimplemented,

    // 98 - temporary filler
    unimplemented,

    // 99 - temporary filler
    unimplemented,

    // 100 - temporary filler
    unimplemented,

    // 101 - temporary filler
    unimplemented,

    // 102 - temporary filler
    unimplemented,

    // 103 - temporary filler
    unimplemented,

    // 104 - temporary filler
    unimplemented,

    // 105 - temporary filler
    unimplemented,

    // 106 - temporary filler
    unimplemented,

    // 107 - temporary filler
    unimplemented,

    // 108 - temporary filler
    unimplemented,

    // 109 - temporary filler
    unimplemented,

    // 110 - temporary filler
    unimplemented,

    // 111 - temporary filler
    unimplemented,

    // 112 - temporary filler
    unimplemented,

    // 113 - temporary filler
    unimplemented,

    // 114 - temporary filler
    unimplemented,

    // 115 - temporary filler
    unimplemented,

    // 116 - temporary filler
    unimplemented,

    // 117 - temporary filler
    unimplemented,

    // 118 - temporary filler
    unimplemented,

    // 119 - temporary filler
    unimplemented,

    // 120 - temporary filler
    unimplemented,

    // 121 - temporary filler
    unimplemented,

    // 122 - temporary filler
    unimplemented,

    // 123 - temporary filler
    unimplemented,

    // 124 - temporary filler
    unimplemented,

    // 125 - temporary filler
    unimplemented,

    // 126 - temporary filler
    unimplemented,

    // 127 - temporary filler
    unimplemented,

    // 128 - temporary filler
    unimplemented,

    // 129 - temporary filler
    unimplemented,

    // 130 - temporary filler
    unimplemented,

    // 131 - temporary filler
    unimplemented,

    // 132 - temporary filler
    unimplemented,

    // 133 - temporary filler
    unimplemented,

    // 134 - temporary filler
    unimplemented,

    // 135 - temporary filler
    unimplemented,

    // 136 - temporary filler
    unimplemented,

    // 137 - temporary filler
    unimplemented,

    // 138 - temporary filler
    unimplemented,

    // 139 - temporary filler
    unimplemented,

    // 140 - temporary filler
    unimplemented,

    // 141 - temporary filler
    unimplemented,

    // 142 - temporary filler
    unimplemented,

    // 143 - temporary filler
    unimplemented,

    // 144 - temporary filler
    unimplemented,

    // 145 - temporary filler
    unimplemented,

    // 146 - temporary filler
    unimplemented,

    // 147 - temporary filler
    unimplemented,

    // 148 - temporary filler
    unimplemented,

    // 149 - temporary filler
    unimplemented,

    // 150 - temporary filler
    unimplemented,

    // 151 - temporary filler
    unimplemented,

    // 152 - temporary filler
    unimplemented,

    // 153 - temporary filler
    unimplemented,

    // 154 - temporary filler
    unimplemented,

    // 155 - temporary filler
    unimplemented,

    // 156 - temporary filler
    unimplemented,

    // 157 - temporary filler
    unimplemented,

    // 158 - temporary filler
    unimplemented,

    // 159 - temporary filler
    unimplemented,

    // 160 - temporary filler
    unimplemented,

    // 161 - temporary filler
    unimplemented,

    // 162 - temporary filler
    unimplemented,

    // 163 - temporary filler
    unimplemented,

    // 164 - temporary filler
    unimplemented,

    // 165 - temporary filler
    unimplemented,

    // 166 - temporary filler
    unimplemented,

    // 167 - temporary filler
    unimplemented,

    // 168 - temporary filler
    unimplemented,

    // 169 - temporary filler
    unimplemented,

    // 170 - temporary filler
    unimplemented,

    // 171 - temporary filler
    unimplemented,

    // 172 - temporary filler
    unimplemented,

    // 173 - temporary filler
    unimplemented,

    // 174 - temporary filler
    unimplemented,

    // 175 - temporary filler
    unimplemented,

    // 176 - temporary filler
    unimplemented,

    // 177 - temporary filler
    unimplemented,

    // 178 - temporary filler
    unimplemented,

    // 179 - temporary filler
    unimplemented,

    // 180 - temporary filler
    unimplemented,

    // 181 - temporary filler
    unimplemented,

    // 182 - temporary filler
    unimplemented,

    // 183 - temporary filler
    unimplemented,

    // 184 - temporary filler
    unimplemented,

    // 185 - temporary filler
    unimplemented,

    // 186 - temporary filler
    unimplemented,

    // 187 - temporary filler
    unimplemented,

    // 188 - temporary filler
    unimplemented,

    // 189 - temporary filler
    unimplemented,

    // 190 - temporary filler
    unimplemented,

    // 191 - temporary filler
    unimplemented,

    // 192 - temporary filler
    unimplemented,

    // 193 - temporary filler
    unimplemented,

    // 194 - temporary filler
    unimplemented,

    // 195 - temporary filler
    unimplemented,

    // 196 - temporary filler
    unimplemented,

    // 197 - temporary filler
    unimplemented,

    // 198 - temporary filler
    unimplemented,

    // 199 - temporary filler
    unimplemented,

    // 200 - temporary filler
    unimplemented,

    // 201 - temporary filler
    unimplemented,

    // 202 - temporary filler
    unimplemented,

    // 203 - temporary filler
    unimplemented,

    // 204 - temporary filler
    unimplemented,

    // 205 - temporary filler
    unimplemented,

    // 206 - temporary filler
    unimplemented,

    // 207 - temporary filler
    unimplemented,

    // 208 - temporary filler
    unimplemented,

    // 209 - temporary filler
    unimplemented,

    // 210 - temporary filler
    unimplemented,

    // 211 - temporary filler
    unimplemented,

    // 212 - temporary filler
    unimplemented,

    // 213 - temporary filler
    unimplemented,

    // 214 - temporary filler
    unimplemented,

    // 215 - temporary filler
    unimplemented,

    // 216 - temporary filler
    unimplemented,

    // 217 - temporary filler
    unimplemented,

    // 218 - temporary filler
    unimplemented,

    // 219 - temporary filler
    unimplemented,

    // 220 - temporary filler
    unimplemented,

    // 221 - temporary filler
    unimplemented,

    // 222 - temporary filler
    unimplemented,

    // 223 - temporary filler
    unimplemented,

    // 224 - temporary filler
    unimplemented,

    // 225 - temporary filler
    unimplemented,

    // 226 - temporary filler
    unimplemented,

    // 227 - temporary filler
    unimplemented,

    // 228 - temporary filler
    unimplemented,

    // 229 - temporary filler
    unimplemented,

    // 230 - temporary filler
    unimplemented,

    // 231 - temporary filler
    unimplemented,

    // 232 - temporary filler
    unimplemented,

    // 233 - temporary filler
    unimplemented,

    // 234 - temporary filler
    unimplemented,

    // 235 - temporary filler
    unimplemented,

    // 236 - temporary filler
    unimplemented,

    // 237 - temporary filler
    unimplemented,

    // 238 - temporary filler
    unimplemented,

    // 239 - temporary filler
    unimplemented,

    // 240 - temporary filler
    unimplemented,

    // 241 - temporary filler
    unimplemented,

    // 242 - temporary filler
    unimplemented,

    // 243 - temporary filler
    unimplemented,

    // 244 - temporary filler
    unimplemented,

    // 245 - temporary filler
    unimplemented,

    // 246 - temporary filler
    unimplemented,

    // 247 - temporary filler
    unimplemented,

    // 248 - temporary filler
    unimplemented,

    // 249 - temporary filler
    unimplemented,

    // 250 - temporary filler
    unimplemented,

    // 251 - temporary filler
    unimplemented,

    // 252 - temporary filler
    unimplemented,

    // 253 - temporary filler
    unimplemented,

    // 254 - temporary filler
    unimplemented,

    // 255 - temporary filler
    unimplemented,

    // 256 - temporary filler
    unimplemented,

    // 257 - temporary filler
    unimplemented,

    // 258 - temporary filler
    unimplemented,

    // 259 - temporary filler
    unimplemented,

    // 260 - temporary filler
    unimplemented,

    // 261 - temporary filler
    unimplemented,

    // 262 - temporary filler
    unimplemented,

    // 263 - temporary filler
    unimplemented,

    // 264 - temporary filler
    unimplemented,

    // 265 - temporary filler
    unimplemented,

    // 266 - temporary filler
    unimplemented,

    // 267 - temporary filler
    unimplemented,

    // 268 - temporary filler
    unimplemented,

    // 269 - temporary filler
    unimplemented,

    // 270 - temporary filler
    unimplemented,

    // 271 - temporary filler
    unimplemented,

    // 272 - temporary filler
    unimplemented,

    // 273 - temporary filler
    unimplemented,

    // 274 - temporary filler
    unimplemented,

    // 275 - temporary filler
    unimplemented,

    // 276 - temporary filler
    unimplemented,

    // 277 - temporary filler
    unimplemented,

    // 278 - temporary filler
    unimplemented,

    // 279 - temporary filler
    unimplemented,

    // 280 - temporary filler
    unimplemented,

    // 281 - temporary filler
    unimplemented,

    // 282 - temporary filler
    unimplemented,

    // 283 - temporary filler
    unimplemented,

    // 284 - temporary filler
    unimplemented,

    // 285 - temporary filler
    unimplemented,

    // 286 - temporary filler
    unimplemented,

    // 287 - temporary filler
    unimplemented,

    // 288 - temporary filler
    unimplemented,

    // 289 - temporary filler
    unimplemented,

    // 290 - temporary filler
    unimplemented,

    // 291 - temporary filler
    unimplemented,

    // 292 - temporary filler
    unimplemented,

    // 293 - temporary filler
    unimplemented,

    // 294 - temporary filler
    unimplemented,

    // 295 - temporary filler
    unimplemented,

    // 296 - temporary filler
    unimplemented,

    // 297 - temporary filler
    unimplemented,

    // 298 - temporary filler
    unimplemented,

    // 299 - temporary filler
    unimplemented,

    // 300 - temporary filler
    unimplemented,

    // 301 - temporary filler
    unimplemented,

    // 302 - temporary filler
    unimplemented,

    // 303 - temporary filler
    unimplemented,

    // 304 - temporary filler
    unimplemented,

    // 305 - temporary filler
    unimplemented,

    // 306 - temporary filler
    unimplemented,

    // 307 - temporary filler
    unimplemented,

    // 308 - temporary filler
    unimplemented,

    // 309 - temporary filler
    unimplemented,

    // 310 - temporary filler
    unimplemented,

    // 311 - temporary filler
    unimplemented,

    // 312 - temporary filler
    unimplemented,

    // 313 - temporary filler
    unimplemented,

    // 314 - temporary filler
    unimplemented,

    // 315 - temporary filler
    unimplemented,

    // 316 - temporary filler
    unimplemented,

    // 317 - temporary filler
    unimplemented,

    // 318 - temporary filler
    unimplemented,

    // 319 - temporary filler
    unimplemented,

    // 320 - temporary filler
    unimplemented,

    // 321 - temporary filler
    unimplemented,

    // 322 - temporary filler
    unimplemented,

    // 323 - temporary filler
    unimplemented,

    // 324 - temporary filler
    unimplemented,

    // 325 - temporary filler
    unimplemented,

    // 326 - temporary filler
    unimplemented,

    // 327 - temporary filler
    unimplemented,

    // 328 - temporary filler
    unimplemented,

    // 329 - temporary filler
    unimplemented,

    // 330 - temporary filler
    unimplemented,

    // 331 - temporary filler
    unimplemented,

    // 332 - temporary filler
    unimplemented,

    // 333 - temporary filler
    unimplemented,

    // 334 - temporary filler
    unimplemented,

    // 335 - temporary filler
    unimplemented,

    // 336 - temporary filler
    unimplemented,

    // 337 - temporary filler
    unimplemented,

    // 338 - temporary filler
    unimplemented,

    // 339 - temporary filler
    unimplemented,

    // 340 - temporary filler
    unimplemented,

    // 341 - temporary filler
    unimplemented,

    // 342 - temporary filler
    unimplemented,

    // 343 - temporary filler
    unimplemented,

    // 344 - temporary filler
    unimplemented,

    // 345 - temporary filler
    unimplemented,

    // 346 - temporary filler
    unimplemented,

    // 347 - temporary filler
    unimplemented,

    // 348 - temporary filler
    unimplemented,

    // 349 - temporary filler
    unimplemented,

    // 350 - temporary filler
    unimplemented,

    // 351 - temporary filler
    unimplemented,

    // 352 - temporary filler
    unimplemented,

    // 353 - temporary filler
    unimplemented,

    // 354 - temporary filler
    unimplemented,

    // 355 - temporary filler
    unimplemented,

    // 356 - temporary filler
    unimplemented,

    // 357 - temporary filler
    unimplemented,

    // 358 - temporary filler
    unimplemented,

    // 359 - temporary filler
    unimplemented,

    // 360 - temporary filler
    unimplemented,

    // 361 - temporary filler
    unimplemented,

    // 362 - temporary filler
    unimplemented,

    // 363 - temporary filler
    unimplemented,

    // 364 - temporary filler
    unimplemented,

    // 365 - temporary filler
    unimplemented,

    // 366 - temporary filler
    unimplemented,

    // 367 - temporary filler
    unimplemented,

    // 368 - temporary filler
    unimplemented,

    // 369 - temporary filler
    unimplemented,

    // 370 - temporary filler
    unimplemented,

    // 371 - temporary filler
    unimplemented,

    // 372 - temporary filler
    unimplemented,

    // 373 - temporary filler
    unimplemented,

    // 374 - temporary filler
    unimplemented,

    // 375 - temporary filler
    unimplemented,

    // 376 - temporary filler
    unimplemented,

    // 377 - temporary filler
    unimplemented,

    // 378 - temporary filler
    unimplemented,

    // 379 - temporary filler
    unimplemented,

    // 380 - temporary filler
    unimplemented,

    // 381 - temporary filler
    unimplemented,

    // 382 - temporary filler
    unimplemented,

    // 383 - temporary filler
    unimplemented,

    // 384 - temporary filler
    unimplemented,

    // 385 - temporary filler
    unimplemented,

    // 386 - temporary filler
    unimplemented,

    // 387 - temporary filler
    unimplemented,

    // 388 - temporary filler
    unimplemented,

    // 389 - temporary filler
    unimplemented,

    // 390 - temporary filler
    unimplemented,

    // 391 - temporary filler
    unimplemented,

    // 392 - temporary filler
    unimplemented,

    // 393 - temporary filler
    unimplemented,

    // 394 - temporary filler
    unimplemented,

    // 395 - temporary filler
    unimplemented,

    // 396 - temporary filler
    unimplemented,

    // 397 - temporary filler
    unimplemented,

    // 398 - temporary filler
    unimplemented,

    // 399 - temporary filler
    unimplemented,

    // 400 - temporary filler
    unimplemented,

    // 401 - temporary filler
    unimplemented,

    // 402 - temporary filler
    unimplemented,

    // 403 - temporary filler
    unimplemented,

};
