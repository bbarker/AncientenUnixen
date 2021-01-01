use core::slice;

//Reusable type for chars, in xv6 they appear to be
//signed chars by default
#[allow(non_camel_case_types)]
pub type c_char = i8;

#[no_mangle]
pub extern "C" fn strlen(s: *const c_char) -> isize {
    unsafe {
        let mut n = 0;
        while *s.offset(n) != 0 {
            n += 1;
        }
        n
    }
}

#[no_mangle]
pub extern "C" fn strncmp(s: *const c_char, t: *const c_char, n: usize) -> isize {
    //This will give weird results if you pass in a negative number
    let s_slice = unsafe { slice::from_raw_parts(s, n) };
    let t_slice = unsafe { slice::from_raw_parts(t, n) };
    //Iterate though elements of both lists at the same time
    //until we reach a 0
    for (l, r) in s_slice
        .iter()
        //Keep going until we reach the end of s
        .take_while(|&&x| x != 0)
        //Add a null character, take_while will remove the final null character
        //otherwise
        .chain([0].iter())
        .zip(t_slice)
    {
        //Return the difference with unmatching characters
        if l != r {
            return *l as isize - *r as isize;
        }
    }
    0
}
#[no_mangle]
pub extern "C" fn strncpy(s: *mut c_char, t: *const c_char, n: isize) -> *const c_char {
    if n <= 0 {
        return s;
    }

    //Turn both char *'s into slices
    let s_slice = unsafe { slice::from_raw_parts_mut(s, n as usize) };
    let t_slice = unsafe { slice::from_raw_parts(t, n as usize) };
    //Iterate through both slices, copying from t to s
    for (empty, ch) in s_slice.iter_mut().zip(t_slice.iter()) {
        *empty = *ch;
    }

    return s;
}
#[no_mangle]
pub extern "C" fn safestrcpy(s: *mut c_char, t: *const c_char, n: isize) -> *const c_char {
    if n <= 0 {
        return s;
    }
    //Turn both char *'s into slices
    //t_slice is 1 element shorter so we can guarantee there's a zero at
    //the end
    let s_slice = unsafe { slice::from_raw_parts_mut(s, n as usize) };
    let t_slice = unsafe { slice::from_raw_parts(t, (n - 1) as usize) };
    //Iterate through both slices, copying from t to s
    //There's an extra zero at the end of t_slice to guarantee that it is nul-
    //terminated
    for (empty, ch) in s_slice.iter_mut().zip(t_slice.iter().chain([0].iter())) {
        *empty = *ch;
    }

    return s;
}
