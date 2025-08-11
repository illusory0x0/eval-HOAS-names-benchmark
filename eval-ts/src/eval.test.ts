import { test, expect } from 'vitest';
import * as E from './eval'

test('tm_to_string',() => {
  expect(E.tm_to_string(E.five)).toMatchSnapshot()
  expect(E.tm_to_string(E.add)).toMatchSnapshot()
  expect(E.tm_to_string(E.mult)).toMatchSnapshot()
})

test('eval',() => {
  let num_25 = E.App(E.App(E.mult,E.five),E.five)
  let num_25_nf = E.nf(num_25,null)

  expect(E.tm_to_string(num_25_nf)).toMatchSnapshot()
})