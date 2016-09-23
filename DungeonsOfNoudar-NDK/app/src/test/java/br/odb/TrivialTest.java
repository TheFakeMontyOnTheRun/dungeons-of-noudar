package br.odb;


import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertTrue;

/**
 * Created by monty on 20/07/16.
 */
public class TrivialTest {

	private boolean falseValue;

	@Before
	public void setUp() {
		falseValue = true;
	}

	@Test
	public void testSomething() {
		assertTrue(true);
	}

	@Test
	public void mustFail() {
		assertTrue( falseValue );
	}
}
